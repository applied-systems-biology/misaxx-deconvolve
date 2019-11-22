//
// Created by rgerst on 22.11.19.
//

#pragma once

#include <misaxx/core/misa_task.h>

namespace misaxx_deconvolve {
    struct convolve_task : public misaxx::misa_task {
        using misaxx::misa_task::misa_task;

        void work() override;
    };
}




