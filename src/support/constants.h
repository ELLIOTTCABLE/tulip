#pragma once

#define tulip_value_type LLVMInt64Type()

#define tulip_defn_type LLVMFunctionType(tulip_value_type, NULL, 0, false)

#define tulip_lambda_type LLVMFunctionType(tulip_value_type, (LLVMTypeRef*) {tulip_value_type}, 1, false)
