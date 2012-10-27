#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/MyLogSoftMax.c"
#else

static int nn_(MyLogSoftMax_updateOutput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));
  real *input_data, *output_data;
  long nframe = 0, dim = 0;
  long t, d;

  if(input->nDimension == 1)
  {
    nframe = 1;
    dim = input->size[0];
  }
  else if(input->nDimension == 2)
  {
    nframe = input->size[0];
    dim = input->size[1];
  }
  else
    THArgCheck(0, 2, "vector or matrix expected");

  input = THTensor_(newContiguous)(input);
  THTensor_(resizeAs)(output, input);

  input_data = THTensor_(data)(input);
  output_data = THTensor_(data)(output);

#pragma omp parallel for private(t)
  for(t = 0; t < nframe; t++)
  {
    real* ptr_output = output_data + t*dim;
    real* ptr_input = input_data + t*dim;
    
    accreal logsum = 0;
    real maxInput = -THInf;

    for(d = 0; d < dim; d++)
      maxInput = THMax(maxInput, ptr_input[d]);

    for(d = 0; d < dim; d++)
      logsum += THExpMinusApprox(maxInput-ptr_input[d]);
    logsum = maxInput + log(logsum);

    for(d = 0; d < dim; d++)
      ptr_output[d] = ptr_input[d] - logsum;
  }

  THTensor_(free)(input);

  return 1;
}

static int nn_(MyLogSoftMax_updateGradInput)(lua_State *L)
{
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_(Tensor_id));
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_(Tensor_id));
  real *gradInput_data, *gradOutput_data, *output_data;
  long nframe = 0, dim = 0;
  long t, d;

  if(output->nDimension == 1)
  {
    nframe = 1;
    dim = output->size[0];
  }
  else if(output->nDimension == 2)
  {
    nframe = output->size[0];
    dim = output->size[1];
  }
  else
    THError("vector or matrix expected");

  THTensor_(resizeAs)(gradInput, output);
  gradInput_data = THTensor_(data)(gradInput);
  output_data = THTensor_(data)(output);
  gradOutput_data = THTensor_(data)(gradOutput);
  
#pragma omp parallel for private(t)
  for(t = 0; t < nframe; t++)
  {
    real* ptr_output = output_data + t*dim;
    real* ptr_gradOutput  = gradOutput_data  + t*dim;
    real* ptr_gradInput  = gradInput_data  + t*dim;

    accreal sum = 0;
    for(d = 0; d < dim; d++)
      sum += ptr_gradOutput[d];

    for(d = 0; d < dim; d++)
      ptr_gradInput[d] = ptr_gradOutput[d] - THExpMinusApprox(ptr_output[d])*sum;
  }

  return 1;
}

static const struct luaL_Reg nn_(MyLogSoftMax__) [] = {
  {"MyLogSoftMax_updateOutput", nn_(MyLogSoftMax_updateOutput)},
  {"MyLogSoftMax_updateGradInput", nn_(MyLogSoftMax_updateGradInput)},
  {NULL, NULL}
};

void nn_(MyLogSoftMax_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(MyLogSoftMax__), "nn");
  lua_pop(L,1);
}

#endif
