#include <misaxx/core/misa_module_interface.h>
#include <misaxx/imaging/accessors/misa_image_file.h>

namespace misaxx_deconvolve {
    struct module_interface : public misaxx::misa_module_interface {

        misaxx::imaging::misa_image_file m_input_image;
        misaxx::imaging::misa_image_file m_input_psf;
        misaxx::imaging::misa_image_file m_output_convolved;
        misaxx::imaging::misa_image_file m_output_deconvolved;

        void setup() override;
    };
}
