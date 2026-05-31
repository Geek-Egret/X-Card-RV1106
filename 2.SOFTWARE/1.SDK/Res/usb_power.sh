#!/bin/sh

USB_ROLE_STATE=$1
GPIO_NUM=145
GPIO_PATH="/sys/class/gpio/gpio${GPIO_NUM}"

# 检查是否已经 export，如果没有则导出
if [ ! -d "$GPIO_PATH" ]; then
    echo $GPIO_NUM > /sys/class/gpio/export
    # 等待设备节点创建
    sleep 0.1
fi

# 设置方向为输出
if [ ! -f "${GPIO_PATH}/direction" ] || [ "$(cat ${GPIO_PATH}/direction)" != "out" ]; then
    echo out > ${GPIO_PATH}/direction
fi

# 设置电平，修正 if 语法（注意空格）
if [ "$USB_ROLE_STATE" = "host" ]; then
    echo 1 > ${GPIO_PATH}/value
else
    echo 0 > ${GPIO_PATH}/value
fi
