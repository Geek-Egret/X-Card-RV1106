# Luckfox RV1106 SDK 开发全记录

> 整合自 GUIDE.md、DEBUG.md、DEBUG_0.md、KERNEL_BUILD_FIX.md

---

## 目录

1. [设备与连接信息](#1-设备与连接信息)
2. [编译 Kernel 并解决错误](#2-编译-kernel-并解决错误)
3. [Driver (.ko) 编译错误修复](#3-driver-ko-编译错误修复)
4. [defconfig 被错误替换导致内核无法启动](#4-defconfig-被错误替换导致内核无法启动)
5. [AIC8800 WiFi 驱动配置](#5-aic8800-wifi-驱动配置)
6. [SDIO / 设备树 / USB OTG 配置](#6-sdio--设备树--usb-otg-配置)
7. [RNDIS 功能核查](#7-rndis-功能核查)
8. [rootfs 自启动脚本与 S90wlan0 修复](#8-rootfs-自启动脚本与-s90wlan0-修复)
9. [SSH 登录问题与修复](#9-ssh-登录问题与修复)
10. [SHA256 密码 hash 不兼容 (Buildroot + uclibc)](#10-sha256-密码-hash-不兼容-buildroot--uclibc)
11. [编译系统与手动编译命令](#11-编译系统与手动编译命令)
12. [OEM 分区构建](#12-oem-分区构建)
13. [烧录镜像](#13-烧录镜像)
14. [完整修改文件清单](#14-完整修改文件清单)
15. [MaskROM 模式下 download.bin 下载失败 (DDR 污染)](#15-maskrom-模式下-downloadbin-下载失败-ddr-污染)

---

## 1. 设备与连接信息

- **设备**: Luckfox Pico Ultra W (Rockchip RV1106)
- **系统**: Buildroot + BusyBox init
- **内核**: Linux 5.10.160, ARM
- **工具链**: `arm-rockchip830-linux-uclibcgnueabihf-`（外部预编译）
- **连接方式**: ADB over USB / SSH over WiFi

---

## 2. 编译 Kernel 并解决错误

### 2.1 编译命令

```bash
./build.sh kernel      # 只编译内核镜像 (boot.img)，不编译模块
./build.sh driver      # 编译内核模块 (.ko) + 外部驱动
./build.sh all         # 完整构建：uboot + kernel + rootfs + media + app + 打包镜像
```

### 2.2 错误 1：rv1106_codec.c — GPIO descriptor API 未声明

```
error: implicit declaration of function 'gpiod_direction_output'
error: implicit declaration of function 'devm_gpiod_get_optional'
error: 'GPIOD_OUT_LOW' undeclared
```

**原因**: 包含了 `<linux/of_gpio.h>`（旧 API），但代码使用 GPIO descriptor 函数（定义在 `<linux/gpio/consumer.h>`）。

**修复**: `sysdrv/source/kernel/sound/soc/codecs/rv1106_codec.c:12`
```c
// 改前
#include <linux/of_gpio.h>
// 改后
#include <linux/gpio/consumer.h>
```

### 2.3 错误 2：spi-rockchip.c — struct device 没有 pins 成员

```
error: 'struct device' has no member named 'pins'
```

**原因**: `include/linux/device.h` 中 `pins` 成员被 `#ifdef CONFIG_PINCTRL` 包裹，但 `CONFIG_PINCTRL` 未启用（RV1106 内核没有 pinctrl 驱动）。

**修复**: `sysdrv/source/kernel/drivers/spi/spi-rockchip.c:674`，用 `#ifdef` 包裹：
```c
#ifdef CONFIG_PINCTRL
    if (rs->high_speed_state) {
        if (rs->freq > IO_DRIVER_4MA_MAX_SCLK_OUT)
            pinctrl_select_state(rs->dev->pins->p,
                                 rs->high_speed_state);
        else
            pinctrl_select_state(rs->dev->pins->p,
                                 rs->dev->pins->default_state);
    }
#endif
```

`high_speed_state` 的初始化代码使用了 `devm_pinctrl_get()` 和 `pinctrl_lookup_state()`，这些函数在 `CONFIG_PINCTRL` 未设置时有对应的 stub 实现（来自 `<linux/pinctrl/consumer.h>`），会返回 NULL/error，最终 `high_speed_state` 被设为 NULL，运行时不会进入该分支。

### 2.4 警告修复：aic8800 Kconfig 文件 DOS 换行符

```
drivers/net/wireless/aic8800/Kconfig:1:warning: ignoring unsupported character ''
```

**原因**: aic8800 驱动的 Kconfig 文件使用了 DOS 风格 CRLF (`\r\n`) 换行符。

**修复**:
```bash
sed -i 's/\r$//' sysdrv/source/kernel/drivers/net/wireless/aic8800/Kconfig
sed -i 's/\r$//' sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_fdrv/Kconfig
sed -i 's/\r$//' sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_btlpm/Kconfig
```

### 2.5 编译成功验证

```
Kernel: arch/arm/boot/Image.gz is ready
Kernel: arch/arm/boot/zImage is ready
Image:  boot.img (FIT image with Linux kernel, FDT blob and resource) is ready
[build.sh:info] Running build_kernel succeeded.
```

---

## 3. Driver (.ko) 编译错误修复

### 3.1 错误 1：mis5001.c — static 函数未使用

```
error: 'mis5001_runtime_suspend' defined but not used [-Werror=unused-function]
error: 'mis5001_runtime_resume' defined but not used [-Werror=unused-function]
```

**原因**: `CONFIG_PM` 未设置，`SET_RUNTIME_PM_OPS()` 展开为空，导致函数无人引用。

**修复**: `sysdrv/source/kernel/drivers/media/i2c/mis5001.c:1129,1138`
```c
static int __maybe_unused mis5001_runtime_resume(struct device *dev)
static int __maybe_unused mis5001_runtime_suspend(struct device *dev)
```

### 3.2 错误 2：aic8800dc 外部驱动 — CONFIG_AIC_FW_PATH 重复定义

```
error: "CONFIG_AIC_FW_PATH" redefined [-Werror]
```

**原因**: 内核 `autoconf.h` 和外部驱动 Makefile 的 `ccflags-y -D` 同时定义了该宏。

**修复**: 注释掉外部驱动 Makefile 中的 `-D` 行：
```
sysdrv/drv_ko/wifi/aic8800dc/aic8800_bsp/Makefile:6
sysdrv/drv_ko/wifi/aic8800dc/aic8800_fdrv/Makefile:251
```

> **注意**: 后续恢复 defconfig 后，正确 arm config 不提供 `CONFIG_AIC_FW_PATH`，因此需要恢复这两个 Makefile 的 `-D` 定义。

### 3.3 编译输出（25 个 .ko）

内核模块（树内编译）：`cfg80211.ko`, `mac80211.ko`, `sc3336.ko`, `imx415.ko`, `mis5001.ko`, `mia1321.ko`, `v4l2-fwnode.ko`, `gspca_main.ko`, `ipv6.ko`，加密模块等。

外部驱动：`aic8800_bsp.ko`, `aic8800_fdrv.ko`, `aic8800_btlpm.ko`, `mpp_vcodec.ko`, `rockit.ko`。

---

## 4. defconfig 被错误替换导致内核无法启动

### 现象

内核烧录后串口打印 `Starting kernel...` 即卡死，无任何后续输出。

### 原因

`arch/arm/configs/luckfox_rv1106_linux_defconfig` 被 SDK 的 HEAD commit 错误覆盖成了 **x86 平台内核配置**（首行 `# Linux/x86 5.10.160 Kernel Configuration`），共 4731 行。虽说 `ARCH=arm`，但 x86 特有选项会导致生成的 ARM 内核配置异常，内核在极早期崩溃。

### 修复

```bash
git show HEAD~1:sysdrv/source/kernel/arch/arm/configs/luckfox_rv1106_linux_defconfig \
    > sysdrv/source/kernel/arch/arm/configs/luckfox_rv1106_linux_defconfig
```

恢复后 defconfig 以正确的 ARM 选项开头（351 行 miniconfig）。

### 校验关键配置

```
CONFIG_ARM=y
CONFIG_ARCH_ROCKCHIP=y
CONFIG_CPU_V7=y
CONFIG_THUMB2_KERNEL=y
```

---

## 5. AIC8800 WiFi 驱动配置

### 5.1 启用 in-tree 驱动

内核树 `drivers/net/wireless/aic8800/` 原本不参与编译（wireless/Makefile 用了不存在的 `CONFIG_AIC8800` 作为开关）。

**修复**：
1. defconfig 添加：
   ```
   CONFIG_AIC_WLAN_SUPPORT=y
   CONFIG_AIC8800_WLAN_SUPPORT=m
   CONFIG_AIC_FW_PATH="/oem/usr/ko/aic8800"
   ```
2. `drivers/net/wireless/Makefile`：将 `obj-$(CONFIG_AIC8800)` 改为 `obj-$(CONFIG_AIC_WLAN_SUPPORT)`
3. 注释掉 `aic8800_bsp/Makefile` 和 `aic8800_fdrv/Makefile` 中的 `-DCONFIG_AIC_FW_PATH`（避免与 autoconf.h 重复）

### 5.2 固件路径

外部驱动 Makefile 编译时硬编码：
```makefile
CONFIG_AIC_FW_PATH = "/oem/usr/ko/aic8800dc_fw"
```

可通过模块参数覆盖：`insmod aic8800_bsp.ko aic_fw_path=/your/path`

---

## 6. SDIO / 设备树 / USB OTG 配置

### 6.1 SDIO 内核配置

`CONFIG_MMC=y` 已包含 SDIO 支持，无需额外配置：
```
CONFIG_MMC=y
CONFIG_MMC_BLOCK=y
CONFIG_MMC_DW=y
CONFIG_MMC_DW_ROCKCHIP=y
```

### 6.2 设备树中启用 SDIO WiFi

`rv1106g-luckfox-pico-ultra-w.dts` 中 WiFi 相关节点全部被注释。参照 `rv1106g-luckfox-pico-pi-w.dts` 取消注释：

```dts
// 1. sdio_pwrseq 电源序列
sdio_pwrseq: sdio-pwrseq {
    compatible = "mmc-pwrseq-simple";
    reset-gpios = <&gpio1 RK_PA2 GPIO_ACTIVE_LOW>;
};

// 2. SDMMC 控制器挂载 WiFi
&sdmmc {
    max-frequency = <50000000>;
    bus-width = <4>;
    cap-sd-highspeed;
    cap-sdio-irq;
    keep-power-in-suspend;
    non-removable;
    supports-sdio;
    mmc-pwrseq = <&sdio_pwrseq>;
    pinctrl-0 = <&sdmmc0_clk &sdmmc0_cmd &sdmmc0_bus4 &sdmmc0_det>;
    status = "okay";
};

// 3. pinmux
&pinctrl {
    sdmmc0 {
        sdmmc0_det: sdmmc0-det {
            rockchip,pins = <3 RK_PA1 1 &pcfg_pull_down>;
        };
    };
};
```

### 6.3 USB OTG 改为从机模式

```dts
&u2phy       { status = "okay"; };
&u2phy_otg   { status = "okay"; };
&usbdrd      { status = "okay"; };
&usbdrd_dwc3 {
    status = "okay";
    extcon = <&u2phy>;
    dr_mode = "peripheral";
};
```

内核配置 `CONFIG_USB_DWC3_DUAL_ROLE=y` + DTS `peripheral` = 强制从机。

---

## 7. RNDIS 功能核查

### 结论：已默认启用，无需修改

| 配置项 | 值 | 说明 |
|--------|-----|------|
| `CONFIG_USB_GADGET` | `y` | USB Gadget 框架 |
| `CONFIG_USB_CONFIGFS` | `y` | ConfigFS 复合设备 |
| `CONFIG_USB_F_RNDIS` | `y` | RNDIS Function 驱动 |
| `CONFIG_USB_CONFIGFS_RNDIS` | `y` | ConfigFS RNDIS 支持 |
| `CONFIG_USB_DWC3_DUAL_ROLE` | `y` | USB 控制器双角色 |

### 使用方式

```bash
mount -t configfs none /sys/kernel/config
cd /sys/kernel/config/usb_gadget
mkdir g1 && cd g1
echo "0x2207" > idVendor
echo "0x0010" > idProduct
mkdir functions/rndis.usb0
ln -s functions/rndis.usb0 configs/c.1
echo "ff400000.usb" > UDC
```

---

## 8. rootfs 自启动脚本与 S90wlan0 修复

### 8.1 rootfs 自启动架构

`./build.sh firmware` 阶段分 4 层构建启动脚本：

#### 动态生成的脚本

| 脚本 | 生成位置 | 功能 |
|------|---------|------|
| `S20linkmount` | `build.sh:1742` 动态拼接 | 创建 `/dev/block/by-name/*` 软链接 |
| `S21appinit` | `build.sh:1464` 动态写入 | 执行 `/oem/usr/bin/RkLunch.sh` |

#### overlay 叠加的脚本

BoardConfig 配置 `RK_POST_OVERLAY` 叠加：

| 来源 overlay | 脚本 |
|-------------|------|
| `buildroot-rgb` | `S25backlight` |
| `buildroot-init` | `S50sshd`, `S60micinit`, `S99hciinit`, `S99rtcinit`, `S99usb0config`, `S99python` |
| `luckfox-config` | `S99luckfoxconfigload`, `S99luckfoxcustomoverlay` |

#### 预编译脚本

`project/scripts/luckfox-buildroot-oem-pre.sh` 在镜像打包前运行。

### 8.2 S90wlan0 无法使用问题

**根因**：
1. `wpa_supplicant.conf` 语法错误：`ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev` 格式不被 wpa_supplicant v2.6 支持
2. `udhcpc` 阻塞启动：`udhcpc -i wlan0` 前台运行，无网络时无限广播 DHCP Discover，阻塞后续启动流程

**修复**：

| 文件 | 修改 |
|------|------|
| `/etc/wpa_supplicant.conf` | 移除无效的 `ctrl_interface` 行 |
| `/etc/init.d/S90wlan0` | 新建，`-C` 参数传 ctrl_interface，wpa_cli 轮询等待连接，`udhcpc -b` 后台模式，实现 start/stop/restart 接口 |

### 8.3 修改自启动脚本的方法

**推荐方法**：在 overlay 目录添加文件：
```bash
cat > project/cfg/BoardConfig_IPC/overlay/overlay-luckfox-buildroot-init/etc/init.d/S98mystart <<'EOF'
#!/bin/sh
case $1 in
    start)  echo "my custom startup" ;;
    stop)   ;;
esac
EOF
chmod +x project/cfg/BoardConfig_IPC/overlay/overlay-luckfox-buildroot-init/etc/init.d/S98mystart
```

**其他方法**：
- 修改 `S21appinit` 目标：编辑 `project/app/rkipc/rkipc/src/rv1106_ipc/RkLunch.sh`
- 使用预编译脚本：编辑 `project/scripts/luckfox-buildroot-oem-pre.sh`

改完后 `./build.sh firmware` 重新打包即可。

---

## 9. SSH 登录问题与修复

### 9.1 现象

Buildroot 构建的 rootfs，SSH 无法用 `root/geekegret` 登录，密码认证失败。

### 9.2 直接修复（在开发板上操作）

```sh
printf 'geekegret\ngeekegret\n' | passwd root
```

### 9.3 验证

```
$ sshpass -p geekegret ssh root@192.168.1.17 'echo SSH_OK'
SSH_OK
```

- WiFi 接口 (192.168.1.17:22) SSH 登录正常
- USB RNDIS 接口 (172.32.0.x:22) 从宿主机不可达（宿主机端路由限制）
- sshd 启停脚本 `/etc/init.d/S50sshd` 工作正常

### 9.4 sshd 关键配置

```
/etc/ssh/sshd_config:
  PermitRootLogin yes
  PasswordAuthentication yes

/etc/shadow:
  root:$1$8v.w3tyo$ZZODBKlYA3WOlL5mII1kk/:20587::::::  (SHA1 crypt, "geekegret")
```

### 9.5 S50sshd 补充修复

OpenSSH 需要 `/var/empty` 目录且属主为 root。原 overlay 脚本缺少创建逻辑：

```bash
# project/cfg/BoardConfig_IPC/overlay/overlay-luckfox-buildroot-init/etc/init.d/S50sshd
start() {
    mkdir -p /var/empty          # 新增
    chown root:root /var/empty/
    /usr/bin/ssh-keygen -A
    /usr/sbin/sshd
}
```

---

## 10. SHA256 密码 hash 不兼容 (Buildroot + uclibc)

### 10.1 现象

Buildroot 配置了 `BR2_TARGET_GENERIC_PASSWD_SHA256=y`，生成 `/etc/shadow` 时 root 密码使用 **SHA256 哈希**（`$5$` 开头）。SSH 密码认证失败。

### 10.2 根因

SDK 使用**外部预编译工具链**：
```
tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/
```

检查其 uclibc 配置（`sysroot/usr/include/bits/uClibc_config.h`）：

```c
#define __UCLIBC_HAS_CRYPT_IMPL__ 1        // crypt() 函数可用
#undef __UCLIBC_HAS_SHA256_CRYPT_IMPL__    // SHA256 未编译！
#undef __UCLIBC_HAS_SHA512_CRYPT_IMPL__    // SHA512 未编译！
```

SSH 密码认证时，sshd 调用 `crypt(password, shadow_hash)` 验证密码。由于 uclibc 的 `crypt()` 不支持 `$5$` 前缀（只支持 DES 和 MD5 `$1$`），认证失败。

Buildroot 构建时用主机工具的 `crypt()`（glibc，支持 SHA256）生成 hash 写入 `/etc/shadow`，但运行时 uclibc 无法验证 SHA256 hash。

**关键点**：
- 本 SDK 使用**外部工具链**，buildroot 内部的 uclibc 配置不生效
- 预编译工具链的 uclibc 未启用 SHA256/SHA512 crypt
- Buildroot 2023.02 已移除 `BR2_TARGET_GENERIC_PASSWD_MD5`（移入 legacy）
- `build.sh rootfs` 只提取 tarball，overlay 在 `build.sh firmware` 阶段应用

### 10.3 修复方案：overlay 覆盖 /etc/shadow 为 MD5 哈希

```bash
mkdir -p project/cfg/BoardConfig_IPC/overlay/overlay-luckfox-buildroot-shadow/etc

cat > project/cfg/BoardConfig_IPC/overlay/overlay-luckfox-buildroot-shadow/etc/shadow <<'EOF'
root:$1$geekegre$LI.lYDgqLkICH3TlJiR9b0:::::::
daemon:*:::::::
bin:*:::::::
sys:*:::::::
sync:*:::::::
mail:*:::::::
www-data:*:::::::
operator:*:::::::
nobody:*:::::::
EOF
```

密码始终为 `geekegret`，仅算法从 SHA256 改为 uclibc 支持的 MD5。

### 10.4 编译与部署

```bash
./build.sh rootfs    # 重新编译 buildroot
./build.sh firmware  # 重新打包镜像（应用 overlay）
./rkflash.sh         # 烧录
```

### 10.5 长期方案

如需使用 SHA256，在 Buildroot 中使能 uclibc 的 `UCLIBC_HAS_CRYPT_SHA256` 选项后重编。

---

## 11. 编译系统与手动编译命令

### 11.1 为什么 `./build.sh kernel` 不编译 .ko

`arch/arm/Makefile` 中 `.img` 目标：
```makefile
%.img:
ifeq ("$(CONFIG_MODULES)$(MAKE_MODULES)$(srctree)","yy$(objtree)")
    $(Q)$(MAKE) $*.dtb zImage Image.gz modules
else
    $(Q)$(MAKE) $*.dtb zImage Image.gz
endif
```

使用 `O=objs_kernel` 外部构建时 `srctree != objtree`，modules 被跳过。SDK 分层设计：`kernel` 管镜像，`driver` 管模块。

### 11.2 手动编译命令（不用 `./build.sh`）

```bash
KERNEL_SRC=/absolute/path/to/sysdrv/source/kernel
OBJ_DIR=/absolute/path/to/sysdrv/source/objs_kernel
CROSS=arm-rockchip830-linux-uclibcgnueabihf-

# 生成 .config
make -C $KERNEL_SRC ARCH=arm O=$OBJ_DIR CROSS_COMPILE=$CROSS \
    luckfox_rv1106_linux_defconfig

# 合并 wifi fragment
$KERNEL_SRC/scripts/kconfig/merge_config.sh -O $OBJ_DIR \
    $OBJ_DIR/.config $KERNEL_SRC/arch/arm/configs/rv1106-sdiowifi.config

make -C $KERNEL_SRC ARCH=arm O=$OBJ_DIR CROSS_COMPILE=$CROSS olddefconfig

# 编译内核镜像
make -C $KERNEL_SRC ARCH=arm O=$OBJ_DIR CROSS_COMPILE=$CROSS -j$(nproc) \
    rv1106g-luckfox-pico-ultra-w.img BOOT_ITS=$KERNEL_SRC/boot.its

# 编译模块
make -C $KERNEL_SRC ARCH=arm O=$OBJ_DIR CROSS_COMPILE=$CROSS -j$(nproc) modules
```

输出位置：
- `boot.img` → `$OBJ_DIR/boot.img`
- `zImage`  → `$OBJ_DIR/arch/arm/boot/zImage`
- `*.ko`    → 散落在 `$OBJ_DIR/drivers/` 下

### 11.3 .ko 模块输出路径

`./build.sh driver` 编译完成后，`.ko` 文件路径：
```
sysdrv/out/kernel_drv_ko/
  → output/out/sysdrv_out/kernel_drv_ko/
```

最终打包 OEM 镜像时复制到 `oem/usr/ko/`。

生成流程：
1. `make modules` → 编译树内 =m 的 .ko
2. `make modules_install INSTALL_MOD_PATH=objs_kernel/drv_ko` → 安装到临时目录
3. `find ... -name "*.ko" -exec cp {} sysdrv/out/kernel_drv_ko \;` → 集中收集
4. `make -C sysdrv/drv_ko` → 编译外部驱动，也拷贝到同一个目录
5. `MAROC_COPY_PKG_TO_SYSDRV_OUTPUT` → 拷贝到 `output/out/sysdrv_out/`

---

## 12. OEM 分区构建

`./build.sh firmware`（或 `all`）中调用 `__PACKAGE_OEM()` 打包 OEM：

| 来源 | 目标 |
|------|------|
| `sysdrv/out/kernel_drv_ko/` | `oem/usr/ko/` |
| `output/out/app_out/{bin,lib,etc}` | `oem/usr/` |
| `output/out/media_out/{bin,lib,share}` | `oem/usr/` |
| IQ 校准文件 | `oem/usr/share/iqfiles/` |

当前配置 `RK_BUILD_APP_TO_OEM_PARTITION=y`，调用 `build_mkimg oem` 生成独立 ext4 镜像 `output/image/oem.img`。

---

## 13. 烧录镜像

```bash
./rkflash.sh          # 烧录全部镜像
./rkflash.sh boot     # 只烧 boot.img（内核+dtb）
./rkflash.sh rootfs   # 只烧根文件系统
```

通过 USB 连接板子，进入 loader 模式（按住按键上电），用 `tools/linux/Linux_Upgrade_Tool/upgrade_tool` 烧录。

---

## 14. 完整修改文件清单

| 文件 | 修改内容 |
|------|----------|
| `sysdrv/source/kernel/sound/soc/codecs/rv1106_codec.c` | `of_gpio.h` → `gpio/consumer.h` |
| `sysdrv/source/kernel/drivers/spi/spi-rockchip.c` | pinctrl 代码加 `#ifdef CONFIG_PINCTRL` |
| `sysdrv/source/kernel/drivers/media/i2c/mis5001.c` | suspend/resume 加 `__maybe_unused` |
| `sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_bsp/aicsdio.c` | suspend/resume 加 `__maybe_unused` |
| `sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_fdrv/aicwf_sdio.c` | suspend/resume 加 `__maybe_unused` |
| `sysdrv/source/kernel/drivers/net/wireless/Kconfig` (aic8800) | 移除 CRLF 换行符 |
| `sysdrv/source/kernel/drivers/net/wireless/aic8800/*/Kconfig` (3 个) | 移除 CRLF 换行符 |
| `sysdrv/source/kernel/drivers/net/wireless/Makefile` | `CONFIG_AIC8800` → `CONFIG_AIC_WLAN_SUPPORT` |
| `sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_bsp/Makefile` | 注释 `-DCONFIG_AIC_FW_PATH` |
| `sysdrv/source/kernel/drivers/net/wireless/aic8800/aic8800_fdrv/Makefile` | 注释 `-DCONFIG_AIC_FW_PATH` |
| `sysdrv/drv_ko/wifi/aic8800dc/aic8800_bsp/Makefile` | 注释/恢复 `-DCONFIG_AIC_FW_PATH` (视 defconfig 而定) |
| `sysdrv/drv_ko/wifi/aic8800dc/aic8800_fdrv/Makefile` | 注释/恢复 `-DCONFIG_AIC_FW_PATH` (视 defconfig 而定) |
| `sysdrv/source/kernel/arch/arm/configs/luckfox_rv1106_linux_defconfig` | 恢复原始 ARM defconfig + 添加 AIC_WLAN_SUPPORT |
| `sysdrv/source/kernel/arch/arm/boot/dts/rv1106g-luckfox-pico-ultra-w.dts` | 启用 SDIO WiFi + USB 从机模式 |
| `project/cfg/BoardConfig_IPC/overlay/*/etc/shadow` | MD5 哈希覆盖 SHA256 |
| `project/cfg/BoardConfig_IPC/overlay/*/etc/init.d/S50sshd` | 添加 `mkdir -p /var/empty` |
| `project/cfg/BoardConfig_IPC/overlay/*/etc/init.d/S90wlan0` | 新建，WiFi 自启动脚本 |
| `/etc/wpa_supplicant.conf` | 移除无效 `ctrl_interface` 行 |
| `/etc/shadow` | root 密码 hash 改为 MD5（`$1$`） |

---

## 15. MaskROM 模式下 download.bin 下载失败 (DDR 污染)

**日期**: 2026-05-28
**工具**: `upgrade_tool` v2.17 / `rkdeveloptool` v1.2

### 15.1 现象

板子显示为 MaskROM 模式（`lsusb` 可见 `2207:110c`，`upgrade_tool LD` 显示 `Mode=Maskrom`），但执行 `upgrade_tool DB download.bin` 时报错：

```
Download boot failed!
Note:please check ddr,please reset device and retry
```

或者是：

```
Download Boot Fail
```

`upgrade_tool TD`、`upgrade_tool UF` 同样失败。

### 15.2 根因：eMMC 中有效固件导致 DDR 被提前初始化

#### 启动流程

```
上电 → MaskROM(芯片ROM固化) → 扫描启动源 → 加载 SPL → DDR初始化 → U-Boot → Kernel
```

#### MaskROM 的启动源扫描顺序

```
1. SPI Flash
2. eMMC           ← 如果 eMMC 中有有效固件，MaskROM 会加载它
3. SD Card
4. USB 从设备模式  ← 只有前面全部找不到时，才进入 USB 等待
```

#### DDR 污染机制

```
上电
 │
 ├─ MaskROM 扫描 eMMC
 │     └─ 发现有效固件 → 加载 U-Boot SPL 到内部 SRAM
 │           │
 │           └─ U-Boot SPL 初始化 DDR 控制器 (DDR4 @ 792MHz)
 │                 │                          ↑
 │                 │              DDR PHY PLL 锁定、寄存器被改写
 │                 │
 │                 └─ SPL 执行失败/超时/看门狗复位
 │                       │
 │                       └─ 芯片复位
 │                             │
 │                             └─ 再次进入 MaskROM → USB 等待 (2207:110c)
 │                                   │
 │                                   │   ⚠ DDR 控制器寄存器未复位！
 │                                   │   PLL 仍在锁定状态
 │                                   │   PHY 处于已训练状态
 │                                   │
 │                                   └─ 你发送 download.bin
 │                                         │
 │                                         └─ MiniLoader 尝试初始化 DDR
 │                                               │
 │                                               └─ 寄存器写入冲突
 │                                                  DDR PHY 重新训练失败
 │                                                  → "check ddr"
```

#### 关键点

Rockchip 芯片的 MaskROM 复位 **不会复位 DDR 控制器**。DDR PHY 的 PLL 和寄存器在 SPL 初始化后保持其状态，即使芯片随后看门狗复位回到 MaskROM 也不清零。MiniLoader 内部的 DDR 初始化代码假设 DDR 控制器处于 POR（上电复位）默认状态，当遇到已在运行的 DDR PHY 时，寄存器写入序列失败，DDR 训练超时。

```
状态 A（正常，eMMC 空）:              状态 B（异常，eMMC 有固件）:
┌──────────────┐                     ┌──────────────┐
│ DDR 控制器    │                     │ DDR 控制器    │
│ 处于复位状态   │                     │ 已被 U-Boot   │
│ 寄存器为默认值 │                     │ SPL 配置过    │
│              │                     │ PLL 已锁定    │
│ 可被 MiniLoader│                    │ PHY 已训练    │
│ 正常初始化     │                     │ 重新初始化    │
│              │                     │ → 冲突失败    │
└──────────────┘                     └──────────────┘
```

### 15.3 解决方案

**方案 1：按住 BOOT 键上电（推荐）**

```
1. 拔掉 USB，等待 10-15 秒（让电容放电，DDR PHY 回到复位）
2. 按住板子上的 BOOT 按钮不放
3. 插入 USB
4. 等待 2 秒
5. 松开 BOOT 按钮
6. 执行 upgrade_tool DB download.bin → 正常
```

**原理**：BOOT 按钮短接 eMMC CLK 到 GND，eMMC 无法响应 MaskROM 的读命令，MaskROM 认为 eMMC 不存在，跳过它直接进入 USB 等待模式。全程 DDR 控制器保持 POR 默认状态。

```
正常上电:                           按住 BOOT 上电:
┌─────────┐                        ┌─────────┐
│  eMMC   │                        │  eMMC   │
│  CLK 正常 │                       │  CLK → GND │ ← 短接到地
│         │                        │  无时钟信号 │
│ MaskROM │                        │ MaskROM    │
│ 能读出  │                        │ 读 eMMC    │
│ U-Boot  │                        │ → 失败!    │
└─────────┘                        └─────────┘
                                         │
                                         ▼
                                  MaskROM 跳过 eMMC
                                  DDR 保持复位状态
                                         │
                                         ▼
                                  进入 USB 等待模式
                                  DDR 干净 → download.bin 正常
```

**方案 2：短接 eMMC CLK**

如果没有 BOOT 按钮，直接短接 eMMC 芯片的 CLK 引脚到 GND 再上电。

### 15.4 复现条件

| 条件 | 是否复现 |
|------|----------|
| eMMC 中无有效固件 | ❌ 不复现 |
| eMMC 中有有效固件，上电直接进 MaskROM | ✅ 必现 |
| 按住 BOOT 键上电进 MaskROM | ❌ 不复现 |
| 擦除 eMMC 后立即重新进 MaskROM | ❌ 不复现（eMMC 已空） |
| 烧录完成后不拔 USB，直接再操作 | ❌ 不复现（DDR 由 MiniLoader 管理） |

### 15.5 排查记录

| 时间 | 操作 | 结果 |
|------|------|------|
| 首次上电 | `upgrade_tool DB` | ✅ Download boot ok |
| 首次上电 | `upgrade_tool UF` 烧录固件 | ✅ 成功 |
| 烧录后重新上电（未按 BOOT） | `upgrade_tool DB` | ❌ check ddr |
| 拔电 3 秒重插（未按 BOOT） | `upgrade_tool DB` | ❌ check ddr |
| 尝试旧版本 download.bin | `upgrade_tool DB` | ❌ check ddr |
| 尝试 rkdeveloptool db | `rkdeveloptool db` | ❌ Opening loader failed |
| **按住 BOOT 键上电** | `upgrade_tool DB` | ✅ Download boot ok |

### 15.6 相关命令

```bash
# 检查板子状态
lsusb | grep 2207
upgrade_tool LD

# 正常操作流程（必须先按 BOOT 上电）
upgrade_tool DB download.bin    # 下载 MiniLoader
upgrade_tool UF update.img      # 烧录固件
upgrade_tool EF download.bin    # 擦除全部 flash
upgrade_tool RCI                # 读芯片信息
upgrade_tool RFI                # 读 Flash 信息
```
