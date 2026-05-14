# 使用说明
1. 进入SDK目录
2. ./build.sh选择custom
3. ./build.sh all
4. 烧录
5. USB连接卡片电脑
5. 进入Res目录，运行setup.sh将网卡模块固件推送到/oem/usr/ko
# 修改说明
1. 使用的设备树、kernel defconfig、buildroot defconfig详见`SDK/project/cfg/BoardConfig_IPC/BoardConfig-EMMC-Buildroot-RV1106_Geek_Egret-IPC.mk`
2. buildroot defconfig路径`SDK/sysdrv/tools/board/buildroot/geekegret_card06_defconfig`
3. wpa_supplicant.conf路径`SDK/project/app/wifi_app/wpa_supplicant.conf`
# 账号密码
账号：root，密码：geekegret


