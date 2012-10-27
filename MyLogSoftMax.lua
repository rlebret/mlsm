local MyLogSoftMax = torch.class('nn.MyLogSoftMax', 'nn.Module')

function MyLogSoftMax:updateOutput(input)
   return input.nn.MyLogSoftMax_updateOutput(self, input)
end

function MyLogSoftMax:updateGradInput(input, gradOutput)
   return input.nn.MyLogSoftMax_updateGradInput(self, input, gradOutput)
end
