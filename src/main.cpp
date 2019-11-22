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