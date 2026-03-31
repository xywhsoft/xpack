# xpkcon

`xpkcon` 是 xPack 的控制台工具，命令风格参考旧版 `xpkcon` 和 7z 控制台程序。

## 命令

- `a` / `add`: 添加文件
- `u` / `update`: 更新文件，路径包和索引包在目标不存在时会自动追加
- `x`: 按包内路径解压
- `e`: 平铺解压
- `l` / `list`: 列表
- `t` / `test`: 校验
- `d` / `delete`: 删除
- `i` / `info`: 查看包信息
- `b` / `build`: 重建包，回收空洞，并应用 `solid/volume` 目标布局

## 选项

- `-t<type>`: `core/index/linux/win32`
- `-l<level>`: 压缩级别 `0..15`
- `-s<0|1>`: solid 模式目标值
- `-V<size>`: 分卷大小，例如 `64M`
- `-o<dir>`: 解压输出目录
- `-w<path>`: `xpkBuild` 使用的临时输出路径
- `-r`: 递归目录
- `-v`: 详细输出
- `-y`: 删除时自动确认

## 映射语法

- Path 包支持 `包内路径::本地文件`
- Index 包支持 `索引::本地文件`
- Core 包更新时支持 `位置::本地文件`

## 构建

Windows:

```bat
cd tools\xpkcon
build_x64.bat
```

或：

```bat
cd tools\xpkcon
build_GCC_x64.bat
```

Linux:

```sh
cd tools/xpkcon
sh build.sh
```

默认输出:

- Windows: `release/x64/xpkcon.exe`
- Linux: `release/x64/xpkcon`
