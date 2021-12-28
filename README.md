# 硬盘烧录工具
```
true
directory fs0:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata
directory fs1:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata
directory fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata
cat -h fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\\bbpv-00000065-D3112429000026E279293906
cat -h fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\\seal-00008015-00184440289B002E
```


//iboot 指令
```
WLACC
syscfg print SrNm
syscfg print Mod#
syscfg print Regn
syscfg print RMd#
syscfg print MLB#
syscfg print WMac
syscfg print BMac
syscfg print EMac
syscfg print CLHS
syscfg print DClr
syscfg print NvSn
syscfg print NSrN
syscfg print LCM#
syscfg print Batt
syscfg print BCMS
syscfg print FCMS
syscfg print MtSN
syscfg print WCAL
nandsize
```

## 命令下发
```
echo WLACC > /tmp/text
echo "syscfg print SrNm" > /tmp/text
echo "syscfg print Mod#" > /tmp/text
echo "syscfg print RMd#" > /tmp/text
echo "syscfg print MLB#" > /tmp/text
echo "syscfg print WMac" > /tmp/text
echo "syscfg print BMac" > /tmp/text
echo "syscfg print EMac" > /tmp/text
echo "syscfg print CLHS" > /tmp/text
echo "syscfg print DClr" > /tmp/text
echo "syscfg print NvSn" > /tmp/text
echo "syscfg print NSrN" > /tmp/text
echo "syscfg print LCM#" > /tmp/text
echo "syscfg print BCMS" > /tmp/text
echo "syscfg print FCMS" > /tmp/text
echo "syscfg print MtSN" > /tmp/text
echo "syscfg print WCAL" > /tmp/text
echo "nandsize" > /tmp/text

echo "directory fs0:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata" > /tmp/text
echo "directory fs1:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata" > /tmp/text
echo "directory fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata" > /tmp/text
echo "cat -h fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\\bbpv-00000065-D3112429000026E279293906" > /tmp/text
echo "cat -h fs2:\\FactoryData\\System\\Library\\Caches\\com.apple.factorydata\\seal-00008015-00184440289B002E" > /tmp/text
```