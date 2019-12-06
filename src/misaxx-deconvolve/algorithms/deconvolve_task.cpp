//
// Created by rgerst on 22.11.19.
//

#include "deconvolve_task.h"
#include <opencv2/opencv.hpp>
#include <misaxx-deconvolve/module_interface.h>
#include <cmath>
#include <misaxx/imaging/utils/tiffio.h>

using namespace misaxx_deconvolve;

namespace cv::images {
    using grayscale8u = cv::Mat1b;
    using grayscale32f = cv::Mat1f;
    using mask = cv::Mat1b;
    using labels = cv::Mat1i;
    using complex = cv::Mat2f;
}

namespace {
    cv::Size get_fft_size(const cv::Mat &img, const cv::Mat &kernel) {
        return cv::Size(img.size().width + kernel.size().width - 1,
                        img.size().height + kernel.size().height - 1);
    }

    cv::images::grayscale32f
    fftunpad(const cv::images::grayscale32f &deconvolved, const cv::Size &target_size, const cv::Size &source_size) {
        int bleft = deconvolved.size().width / 2 - source_size.width / 2;
        int btop = deconvolved.size().height / 2 - source_size.height / 2;
        cv::Rect roi{bleft, btop, source_size.width, source_size.height};
        cv::images::grayscale32f result{source_size, 0};
        deconvolved(roi).copyTo(result);
        return result;
    }

    cv::images::grayscale32f
    fftpad(const cv::images::grayscale32f &img, const cv::Size &target_size, bool shift = false) {
        cv::Size ap{};
        if (img.size().width % 2 == 0)
            ap.width = 1;
        if (img.size().height % 2 == 0)
            ap.height = 1;

        cv::Size c{};
        c.width = (target_size.width - img.size().width - ap.width) / 2;
        c.height = (target_size.height - img.size().height - ap.height) / 2;

        int bleft = c.width;
        int btop = c.height;
        int bright = c.width + ap.width;
        int bbottom = c.height + ap.height;

        // Further pad to optimal FFT size
        {
            int currentWidth = bleft + bright + img.size().width;
            int currentHeight = btop + bbottom + img.size().height;
            int optimalWidth = cv::getOptimalDFTSize(currentWidth);
            int optimalHeight = cv::getOptimalDFTSize(currentHeight);

            // Add to padding
            bright += optimalWidth - currentWidth;
            bbottom += optimalHeight - currentHeight;
        }

        cv::images::grayscale32f padded = img;
        cv::copyMakeBorder(img, padded, btop, bbottom, bleft, bright, cv::BORDER_CONSTANT, cv::Scalar::all(0));


        if (shift) {
            const int sx = padded.size().width / 2;
            const int sy = padded.size().height / 2;
            cv::Rect A{0, 0, sx, sy};
            cv::Rect B{sx, 0, sx, sy};
            cv::Rect C{0, sy, sx, sy};
            cv::Rect D{sx, sy, sx, sy};

            auto src = padded.clone();
            src(A).copyTo(padded(D));
            src(B).copyTo(padded(C));
            src(C).copyTo(padded(B));
            src(D).copyTo(padded(A));
        }

        return padded;
    }

    cv::images::complex fft(const cv::images::grayscale32f &padded) {
        cv::images::complex result{padded.size(), cv::Vec2f{0, 0}};
        cv::dft(padded, result, cv::DFT_COMPLEX_OUTPUT);
        return result;
    }

    void normalize(cv::images::grayscale32f &img) {
        double vmin = 0;
        double vmax = 0;
        cv::minMaxLoc(img, &vmin, &vmax);

        for (int y = 0; y < img.rows; ++y) {
            float *row = img[y];
            for (int x = 0; x < img.cols; ++x) {
                row[x] = static_cast<float>((row[x] - vmin) / (vmax - vmin));
            }
        }
    }

    void clamp(cv::images::grayscale32f &img) {
        for (int y = 0; y < img.rows; ++y) {
            float *row = img[y];
            for (int x = 0; x < img.cols; ++x) {
                row[x] = std::min(std::max(row[x], 0.0f), 1.0f);
            }
        }
    }

    cv::images::grayscale32f ifft(const cv::images::complex &fft) {
        cv::images::grayscale32f result{fft.size(), 0};
        cv::images::complex tmp{fft.size(), 0};
        cv::dft(fft, tmp, cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        cv::mixChannels({tmp}, {result}, {0, 0});
        cv::flip(result.clone(), result, -1);
        return result;
    }

    cv::images::grayscale32f get_laplacian8_kernel() {
        cv::images::grayscale32f result{cv::Size(3, 3), 0};
        for (int y = 0; y < result.rows; ++y) {
            float *row = result[y];
            for (int x = 0; x < result.cols; ++x) {
                row[x] = x == 1 && y == 1 ? -1.0f : 1.0f / 8.0f;
            }
        }
        return result;
    }

    cv::images::complex get_laplacian_fft(const cv::Size& unopt_target_size, const cv::Size &target_size) {
        cv::Size quadrant_size{target_size.width / 2 - 1, target_size.height / 2 - 1};

        cv::images::complex quadrant{quadrant_size, cv::Vec2f(0, 0)};
        for (int y = 0; y < quadrant.rows; ++y) {
            cv::Vec2f *row = quadrant[y];
            const float wy = M_PI * y / quadrant_size.height;
            for (int x = 0; x < quadrant.cols; ++x) {
                const float wx = M_PI * x / quadrant_size.width;
                row[x] = cv::Vec2f(wx * wx + wy * wy, 0);
            }
        }

        cv::images::complex result{target_size, cv::Vec2f(0, 0)};

        cv::Size nudge {0, 0};
        cv::copyMakeBorder(quadrant,
                           result,
                           nudge.height,
                           target_size.height - quadrant_size.height + nudge.height,
                           nudge.width,
                           target_size.width - quadrant_size.width + nudge.width,
                           cv::BORDER_REFLECT);

        return result;
    }

    inline cv::Vec2f complex_add(cv::Vec2f a, cv::Vec2f b) {
        return cv::Vec2f{a[0] + b[0], a[1] + b[1]};
    }

    inline cv::Vec2f complex_mul(cv::Vec2f a, cv::Vec2f b) {
        return cv::Vec2f{a[0] * b[0] - a[1] * b[1], a[0] * b[1] + a[1] * b[0]};
    }

    inline cv::Vec2f scalar_mul(cv::Vec2f a, float b) {
        return cv::Vec2f{a[0] * b, a[1] * b};
    }

    inline cv::Vec2f complex_div(cv::Vec2f a, cv::Vec2f b) {
        const float A0 = a[0] * b[0] + a[1] * b[1];
        const float B0 = a[1] * b[0] - a[0] * b[1];
        const float D = b[0] * b[0] + b[1] * b[1];
        return cv::Vec2f{A0 / D, B0 / D};
    }

    inline float complex_abs(cv::Vec2f a) {
        return std::sqrt(a[0] * a[0] + a[1] * a[1]);
    }

    void visualize_fft(const cv::images::complex &fft) {
        cv::images::grayscale32f visualization {fft.size(), 0};
        for(int y = 0; y < visualization.rows; ++y) {
            const cv::Vec2f *rFFT = fft[y];
            float *rVis = visualization[y];
            for(int x = 0; x < visualization.cols; ++x) {
                rVis[x] = complex_abs(rFFT[x]);
            }
        }
    }
}

void deconvolve_task::work() {
    auto module_interface = get_module_as<misaxx_deconvolve::module_interface>();
    auto access_convolved = module_interface->m_output_convolved.access_readonly();
    auto access_psf = module_interface->m_input_psf.access_readonly();

    const float rif_lambda = 0.001f;

    cv::Size target_size = get_fft_size(access_convolved.get(), access_psf.get());
    cv::images::complex H = fft(fftpad(access_psf.get(), target_size, true));
    cv::images::complex Y = fft(fftpad(access_convolved.get(), target_size));
    cv::images::complex L = get_laplacian_fft(target_size, Y.size());
    cv::images::complex X{H.size(), 0};

    for (int y = 0; y < Y.rows; ++y) {
        const cv::Vec2f *rH = H[y];
        const cv::Vec2f *rY = Y[y];
        const cv::Vec2f *rL = L[y];
        cv::Vec2f *rX = X[y];
        for (int x = 0; x < Y.cols; ++x) {
            const cv::Vec2f H2 = complex_mul(rH[x], rH[x]);
            const cv::Vec2f L2 = scalar_mul(complex_mul(rL[x], rL[x]), rif_lambda);
            const cv::Vec2f FA = complex_add(H2, L2);
            const cv::Vec2f FP = complex_div(rH[x], FA);
            const cv::Vec2f D = complex_mul(rY[x], FP);
            rX[x] = D;
        }
    }

    cv::images::grayscale32f deconvolved = fftunpad(ifft(X), target_size, access_convolved.get().size());
//    clamp(deconvolved);
    module_interface->m_output_deconvolved.write(deconvolved);
}
