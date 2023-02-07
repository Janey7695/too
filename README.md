# 它是 What it is?
- `tiny-Thread pOOl(tOO)`是一个简易的线程池库，用以替换我的`tinyblog`项目的libhv库

# 怎么用 How to use?
## 代码编写 code
`tOO`的对外引出的函数只有3个，结构体只有1个,十分简单易用

分别是：
1. `threadpool_t`变量 负责管理整个线程池的数据结构
2. `threadpool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);` 用以创建、初始化一个线程池。参数分别
    * `min_thr_num` 线程池允许的最低存活线程数量
    * `max_thr_num` 线程池允许的最高存活线程数量
    * `queue_max_size` 任务队列允许的最大数量
3. `int threadpool_add_task(threadpool_t *pool,void *(*taskfunction)(void *arg),void *arg);` 用以向线程池中加入任务
    * `*pool` threadpool\_t 变量的指针
    * `*taskfunction` 任务的函数指针，该任务函数必须是非无限的
    * `*arg` 要传递给任务的参数的匿名指针
4. `int threadpool_destroy(threadpool_t *pool);` 用以结束时，销毁整个线程池

### 使用样例 example

``` c
#include <stdio.h>
#include "too.h"

void hello_task(void *arg){
    arg = arg;
    printf("Hello tOO ! \r\n");
}

// ...
int main(){
    threadpool_t *pool= NULL;
    pool = threadpool_create(2,4,20);
    
    //...
    threadpool_add_task(pool,hello_task,NULL);
    //...

    threadpool_destroy(pool);
    return 0;
}

```

## 库的链接 link
### 直接嵌入工程中 emb to project
`tOO`的源文件只有4个，`too.h`,`too_threadpool.c`,`utils.c`,`utils.h` ,如果不介意的话，可以直接将这4个文件夹杂到你的工程中，届时作为工程源文件的一部分参与编译即可，这种方式最简单。

### cmake编译为静、动态库后链接 compile to lib
#### 编译
``` shell
mkdir build
cd build 
cmake ..
cmake --build .
```
之后便会在build文件夹中生成`.so`和`.a`库文件,`tOO`的`CMakeLists.txt`会默认生成静态库和动态库，将生成的库文件进行链接即可
