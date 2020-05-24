npm install --save electron

npm install nan bindings --save

npm install -g node-gyp

配置编译工程
sudo node-gyp configure build

每次修改c++文件后需要，重新编译项目
node-gyp build

如果修改了c++文件结构，还需要重新配置项目
node-gyp configure build


node.js版本不一致
1. Install electron-rebuild
npm install --save electron-rebuild

2. Rebuild
./node_modules/.bin/electron-rebuild