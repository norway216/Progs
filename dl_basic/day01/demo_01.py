import torch
import torch.nn as nn

class SimpleNet(nn.Module):
    def __init__(self, input_size, hidden_size1, hidden_size2, output_size):
        super(SimpleNet, self).__init__()
        
        self.net = nn.Sequential(
            nn.Linear(input_size, hidden_size1),
            nn.ReLU(),
            nn.Linear(hidden_size1, hidden_size2),
            nn.ReLU(),
            nn.Linear(hidden_size2, output_size)
        )

    def forward(self, x):
        return self.net(x)


# 创建模型
model = SimpleNet(input_size=10, hidden_size1=16, hidden_size2=8, output_size=2)

print(model)

# 测试输入
x = torch.randn(4, 10)   # batch_size=4, 输入特征维度=10
y = model(x)

print("输入 x 的形状:", x.shape)
print("输出 y 的形状:", y.shape)
print("输出 y:")
print(y)