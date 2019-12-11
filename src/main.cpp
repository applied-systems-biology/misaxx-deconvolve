/**
 * Copyright by Ruman Gerst
 * Research Group Applied Systems Biology - Head: Prof. Dr. Marc Thilo Figge
 * https://www.leibniz-hki.de/en/applied-systems-biology.html
 * HKI-Center for Systems Biology of Infection
 * Leibniz Institute for Natural Product Research and Infection Biology - Hans Knöll Insitute (HKI)
 * Adolf-Reichwein-Straße 23, 07745 Jena, Germany
 *
 * This code is licensed under BSD 2-Clause
 * See the LICENSE file provided with this code for the full license.
 */

#include <misaxx-deconvolve/module.h>
#include <misaxx-deconvolve/module_info.h>
#include <misaxx/core/runtime/misa_cli.h>

using namespace misaxx;
using namespace misaxx_deconvolve;

int main(int argc, const char** argv) {
    misa_cli cli {};
    cli.set_module_info(misaxx_deconvolve::module_info());
    cli.set_root_module<misaxx_deconvolve::module>("misaxx-deconvolve");
    return cli.prepare_and_run(argc, argv);
}
