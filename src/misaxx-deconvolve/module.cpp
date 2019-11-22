#include <misaxx/core/misa_module.h>
#include <misaxx-deconvolve/module.h>
#include <src/misaxx-deconvolve/algorithms/convolve_task.h>
#include <src/misaxx-deconvolve/algorithms/deconvolve_task.h>

using namespace misaxx_deconvolve;

void module::create_blueprints(blueprint_list &t_blueprints, parameter_list &t_parameters) {
    t_blueprints.add(create_blueprint<convolve_task>("convolve"));
    t_blueprints.add(create_blueprint<deconvolve_task>("deconvolve"));
}

void module::build(const blueprint_builder &t_builder) {
    auto &convolution = t_builder.build<convolve_task>("convolve");
    auto &deconvolution = t_builder.build<deconvolve_task>("deconvolve");
    deconvolution.get_node()->get_dependencies().insert(convolution.get_node());
}
