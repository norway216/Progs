# unet_segmentation.py
import torch
import torch.nn as nn
import torch.nn.functional as F
from torch.utils.data import DataLoader
import torchvision
from torchvision import transforms
import matplotlib.pyplot as plt
import numpy as np

# ---------------- UNet 模型 ----------------
class UNet(nn.Module):
    def __init__(self, in_channels=3, out_channels=3):
        super(UNet, self).__init__()
        self.enc1 = self.conv_block(in_channels, 64)
        self.enc2 = self.conv_block(64,128)
        self.enc3 = self.conv_block(128,256)
        self.pool = nn.MaxPool2d(2)
        self.up2 = nn.ConvTranspose2d(256,128,2,stride=2)
        self.dec2 = self.conv_block(256,128)
        self.up1 = nn.ConvTranspose2d(128,64,2,stride=2)
        self.dec1 = self.conv_block(128,64)
        self.out = nn.Conv2d(64,out_channels,1)

    def conv_block(self, in_c, out_c):
        return nn.Sequential(
            nn.Conv2d(in_c,out_c,3,padding=1),
            nn.ReLU(inplace=True),
            nn.Conv2d(out_c,out_c,3,padding=1),
            nn.ReLU(inplace=True)
        )

    def forward(self,x):
        e1 = self.enc1(x)
        e2 = self.enc2(self.pool(e1))
        e3 = self.enc3(self.pool(e2))
        d2 = self.up2(e3)
        d2 = torch.cat([d2,e2],dim=1)
        d2 = self.dec2(d2)
        d1 = self.up1(d2)
        d1 = torch.cat([d1,e1],dim=1)
        d1 = self.dec1(d1)
        out = self.out(d1)
        return out

# ---------------- 数据集 ----------------
transform = transforms.Compose([
    transforms.Resize((32,32)),
    transforms.ToTensor()
])

train_dataset = torchvision.datasets.CIFAR10(root='./data', train=True,
                                             download=True, transform=transform)
train_loader = DataLoader(train_dataset, batch_size=16, shuffle=True)

# ---------------- 训练 ----------------
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
model = UNet(in_channels=3, out_channels=3).to(device)
optimizer = torch.optim.Adam(model.parameters(), lr=1e-3)
criterion = nn.MSELoss()  # 简化，直接回归像素值

epochs = 2  # 测试用
for epoch in range(epochs):
    model.train()
    total_loss = 0
    for imgs, _ in train_loader:
        imgs = imgs.to(device)
        optimizer.zero_grad()
        out = model(imgs)
        loss = criterion(out, imgs)  # 将自己重建作为简化目标
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    print(f"Epoch {epoch+1}/{epochs}, Loss={total_loss/len(train_loader):.4f}")

# ---------------- 推理 ----------------
model.eval()
test_imgs, _ = next(iter(train_loader))
test_imgs = test_imgs.to(device)
with torch.no_grad():
    pred = model(test_imgs)
pred = pred.cpu()

# ---------------- 展示结果 ----------------
def show_images(imgs, preds, n=4):
    for i in range(n):
        plt.subplot(2,n,i+1)
        plt.imshow(np.transpose(imgs[i].numpy(),(1,2,0)))
        plt.axis('off')
        plt.subplot(2,n,n+i+1)
        plt.imshow(np.clip(np.transpose(preds[i].numpy(),(1,2,0)),0,1))
        plt.axis('off')
    plt.show()

show_images(test_imgs.cpu(), pred)