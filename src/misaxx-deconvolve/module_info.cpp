#include <misaxx/core/module_info.h>
#include <misaxx-deconvolve/module_info.h>

misaxx::misa_module_info misaxx_deconvolve::module_info() {
    misaxx::misa_module_info info;
    info.set_id("misaxx-deconvolve");
    info.set_version("1.0.0");
    info.set_name("misaxx-deconvolve");
    info.set_description("Deconvolution example");
    
    info.add_dependency(misaxx::module_info());
    // TODO: Add dependencies via info.add_dependency()
    return info;
}
