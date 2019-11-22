#include <misaxx/core/misa_module_interface.h>
#include <misaxx-deconvolve/module_interface.h>

using namespace misaxx_deconvolve;

void module_interface::setup() {
    m_input_image.suggest_import_location(filesystem, "in");
    m_input_psf.suggest_import_location(filesystem, "psf");
    m_output_convolved.suggest_export_location(filesystem, "convolved", m_input_image.describe());
    m_output_deconvolved.suggest_export_location(filesystem, "deconvolved", m_input_image.describe());
}
