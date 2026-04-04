#pragma once

#include <stdbool.h>

#include "parsing.h"

bool lambda_normal(Lambda *lambda);
Lambda *lambda_reduce(Lambda *lambda);