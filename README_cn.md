
### [English](./README.md) 

# XSCE

XDP全栈架构如下图所示，其中PCT技术层通过集成沙箱计算、可信计算、联邦学习和MPC等组件支持众多算法和应用。XSCE作为MPC组件集成在XDP系统中。 

![](./docs/img/xdp_arch.png)  

XSCE(XDP Secure Computing Engine)作为统一MPC框架提供最常用的MPC算法，易于被上层应用软件集成。XSCE总体架构如下图所示：

![](./docs/img/xsce_arch.png)  

本次MPC开源PSI和PIR算法，未来几个月里会有更多算法开源。

#### 编译选项
- **编译**  

XSCE在Linux系统上测试通过，当前版本依赖于libOTe库，环境最低配置要求：Ubuntu 20, c++ 17, cmake 3.20, python3, 最少指令集要求: aes sse2 sse3 ssse3 sse4.1 sse4.2 avx avx2 avx256 avx512f pclmul bmi2 adx mmx。  

执行如下命令可编译libOTe、MP-SPDZ和XSCE:

```
./build.py libote spdz xsce  
```
`./build.py libote` 命令会编译libOTe库，并安装到`./third_party/libOTe_libs`路径下，XSCE的编译会依赖该路径下的库, 如果该路径下没有库，XSCE将依赖默认的系统路径 `/usr/local`下的库。  
`./build.py spdz` 命令会编译MP-SPDZ库，并安装到`./lib`路径下面，XSCE的编译会依赖该路径下的库, 如果该路径下没有库，XSCE将依赖默认的系统路径 `/usr/local`下的库。
XSCE编译好的库包括：libtoolkits.a, libcommon.a, libPSI.a, libPIR.a and libMP-SPDZ.a, 这些库会放在源码下的`./lib`路径下, 同时可执行的示例程序pir/psi/arithmetic会编译到build/bin路径下：build/bin/pir, build/bin/psi, build/bin/arithmetic。

- **安装**  

编译好XSCE后，可以执行如下命令将XSCE包含的算法头文件、库文件安装到指定路径下，基于XSCE的二次开发可以通过该路径引用到相关头文件和库：
```
./build.py install   安装头文件和库到默认路径/usr/local
or 
./build.py install=/xx/yy 安装头文件和库到指定路径: /xx/yy
```

- **容器**  

XSCE提供Dockerfile，执行如下命令可以自动构建一个Docker的镜像环境，该环境已编译、安装好XSCE二次开发环境，相关的头文件和库会部署到`/usr/local`路径。 
```
./build.py docker=images
```
构建好Docker镜像后，可以启动该Docker，可直接在该环境下做二次开发，也可以git clone XSCE源码，执行如下命令编译示例程序：`./build.py xsce`。

另外：默认情况下，未使用sudo权限，如果系统需要sudo才有权限，需要在./build.py命令后面添加 `sudo`选项。更多的编译选项可以使用`./build.py help`命令查看。

#### 示例
- **psi 示例**  

编译完成后，在项目根目录下执行下面命令运行示例：  
```
build/bin/psi
```

- **pir 示例**  

编译完成后，在项目根目录下执行下面命令运行示例： 
```
build/bin/pir
or
1. 服务端: build/bin/pir -r 0
2. 客户端: build/bin/pir -r 1
```

- **基础运算算法 示例**  

编译完成后，首先创建测试目录，然后将测试程序和电路文件拷贝到测试目录下。
```shell
mkdir test_dir
cp -rf XSCE_PATH/build/bin/arithmetic XSCE_PATH/src/arithmetic/Programs test_dir/
```
此外，若需要运行三方算法，由于使用诚实多数协议(Shamir)，为确保安全，还需要设置ssl证书，然后将证书拷贝至测试目录。
```shell
cd XSCE_PATH/third_party/MP-SPDZ
./Scripts/setup-ssl.sh [<number of parties> <ssl_dir>]  
如果提示'c_rehash: command not found'，可以进入<ssl_dir>，执行`for file in *.pem; do ln -s "$file" "$(openssl x509 -hash -noout -in "$file")".0; done` shell命令。
mkdir test_dir/Player-Data
cp XSCE_PATH/third_party/MP-SPDZ/ssl_dir/* test_dir/Player-Data/
cd test_dir
```
在测试目录下执行下面命令运行示例(以两方加法为例)：
```
1. party0: ./arithmetic -c 2 -r 0 -p 127.0.0.1:7878 add2
2. party1: ./arithmetic -c 2 -r 1 -p 127.0.0.1:7878 add2
```

- **纵向逻辑回归算法 示例**  

编译完成后，将MP-SPDZ电路文件拷贝到根目录下。
```shell
cp -rf XSCE_PATH/src/arithmetic/Programs XSCE_PATH/
```

```
在根目录下执行下面命令运行示例(：
```
1. party0: build/bin/lr  -r 0 -alg 1
2. party1: build/bin/lr  -r 1 -alg 1
```

#### 算法设计
- **PSI**  

PSI 算法设计实现参考IACR 论文 “Private Set Intersection in the Internet Setting From Lightweight Oblivious PRF”: https://eprint.iacr.org/2020/729

- **PIR**  

PIR 算法实现原理如下:
输入输出：客户端（查询方）使用单个或多个id向服务端（数据方）查询，服务端提供多条文本数据（每个数据包含一个id）, 算法过程：
```
1）客户端用输入id值和服务端的id值进行PSI算法，仅客户端获得psi结果，通过psi结果客户端可以获取查询id对应服务端数据的索引；
2）服务端用随机密钥对每个文本数据进行加密，加密后的密文发送给查询端。服务端的随机密钥是在PSI算法过程中产生的oprf值，PSI算法能够保证客户端PSI算法过程中对于双方交集元素能够产生同样的oprf值；
3）客户端利用PSI算法过程中交集元素的oprf值密钥对查询id对应的密文进行解密得到查询结果。
```

- **基础运算**

算法实现基于[MP-SPDZ开源库](https://github.com/data61/MP-SPDZ)，编写、调优基础算法电路文件，XSCE算法框架执行优化后的电路文件的方式，执行相应算法。
支持以下计算：
- 2方加法(`a+b`)
- 2方乘法(`a*b`)
- 2方比较(`a<b`)
- 2方中位数(`给定两个数组，寻找中位数`)
- 2方方差(`给定两个数组，计算方差`)
- 2方内积(`给定两个数组，计算内积`)
- 3方加法(`a+b+c`)
- 3方乘法(`a*b*c`)
- 3方中位数(`给定三个数组，寻找中位数`)
- 3方方差(`给定三个数组，计算方差`)

两方算法提供如下接口：
- `EXPORT_SYM int runAdd2(SPDZAlg *spdzalg);  //两方加法`
- `EXPORT_SYM int runMul2(SPDZAlg *spdzalg);  //两方乘法`
- `EXPORT_SYM int runCmp2(SPDZAlg *spdzalg);  //两方比较`
- `EXPORT_SYM int runVar2(SPDZAlg *spdzalg);  //两方方差`
- `EXPORT_SYM int runMid2(SPDZAlg *spdzalg);  //两方中位数`
- `EXPORT_SYM int runInnerProd2(SPDZAlg *spdzAlg); //两方内积`

三方算法提供如下接口：
- `EXPORT_SYM int runAdd3(SPDZAlg *spdzalg);  //三方加法`
- `EXPORT_SYM int runMul3(SPDZAlg *spdzalg);  //三方乘法`
- `EXPORT_SYM int runVar3(SPDZAlg *spdzalg);  //三方方差`
- `EXPORT_SYM int runMid3(SPDZAlg *spdzalg);  //三方中位数`


- **逻辑回归纵向**  

算法实现原理如下:
```
输入输出：
1）训练数据：数据Server方(role=0)提供本方样本的标签数据和特征数据，Client方(role=1)提供对应样本的特征数据，两方联合进行纵向逻辑回归.测试数据可选;
2）训练参数：迭代次数、训练batchsize、学习速率;
```

算法过程：
1）梯度训练中，每轮迭代中client方进行参数更新时，需要和server方的标签数据进行mpc计算，通过inner product算法（MP-SPDZ）完成;
2) 训练完成后。server方和client方保存各自特征对应的模型参数结果到输出结果文件中;
2）如果输入参数里面提供了测试数据集，双方对测试数据进行训练质量评估。
```

更多算法将在后续进行支持。

# 开源协议

该项目采用apache 2.0开源协议。
