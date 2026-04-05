#pragma once

#include <stdbool.h>

#include "ast.h"

bool lambda_normal(Lambda *lambda);
Lambda *lambda_reduce(Lambda *lambda);