// TODO arrange in alphabetical order, separated by type
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <aio.h>
#include <semaphore.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#define MAX_PATH 256

size_t buffer_size;          //一次需要读取或者写入的大小
uint64_t  page_size;         //文件读取时的页数
int lock_num_requests;
//信号量通知
sem_t blocking_waiter;

//目录文件路径
char * src;
char * dst;

char *tem_buf;

void aio_read_handler (sigval_t  sigval);
void aio_write_handler(sigval_t signal);
char *format_path(char* path) ;
char* Find_Last_dir_Path(char * path);
int is_exist_dir(char* _dir);
void traverse_dir_copy(char * pdir);
char * split_str(char *path,char *src_path,char **result);

//目录或文件需要复制的结构体对象
typedef struct handler_context
{
    struct aiocb* m_aiocb;      //需要异步操的对象
    size_t m_offset;            //偏移量
    size_t m_file_size;         //文件大小
    int m_src_fd;               //源文件fd
    int m_dst_fd;               //目标文件fd
} handler_context;

/*****************************************************************************
 函 数 名  : aio_read_handler
 功能描述  : aio异步读取函数
 输入参数  : sigval_t  sigval  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
void aio_read_handler (sigval_t  sigval)
{
    size_t nbytes;
    size_t w_nbytes = 0;
    handler_context* hctx = (handler_context*)sigval.sival_ptr;
    if (aio_error(hctx->m_aiocb)) 
    {
        perror("read aio error");
        exit(-1);
    }
    nbytes = aio_return(hctx->m_aiocb);
    int i = 0;
    void * buffer = (void *)hctx->m_aiocb->aio_buf;
    /*w_nbytes = pwrite(hctx->m_dst_fd, buffer, nbytes, hctx->m_offset);
    if (w_nbytes != nbytes) {
    perror("sync write error");
    exit(-1);
    }
    sem_post(&blocking_waiter);*/
    // now send an async write request for the destination file
    // init aiocb struct
    struct aiocb*  w_aiocb = (struct aiocb*)malloc(sizeof(struct aiocb));
    handler_context* w_context = (handler_context *) malloc(sizeof(handler_context));
    bzero ((char *)w_context, sizeof(handler_context));
    bzero ((char *)w_aiocb, sizeof(struct aiocb));
    
    // context to be passed to handler
    w_context->m_aiocb = w_aiocb;
    w_context->m_offset = hctx->m_offset;
    w_context->m_file_size = hctx->m_file_size;
    w_context->m_src_fd = hctx->m_src_fd;
    w_context->m_dst_fd = hctx->m_dst_fd;

    // basic setup
    w_aiocb->aio_fildes = hctx->m_dst_fd;
    w_aiocb->aio_nbytes = nbytes;
    w_aiocb->aio_offset = hctx->m_offset;
    w_aiocb->aio_buf = buffer;

    // thread callback
    w_aiocb->aio_sigevent.sigev_notify = SIGEV_THREAD;
    w_aiocb->aio_sigevent.sigev_notify_function = aio_write_handler;
    w_aiocb->aio_sigevent.sigev_notify_attributes = NULL;
    w_aiocb->aio_sigevent.sigev_value.sival_ptr = (void *)w_context;

    if (aio_write(w_aiocb) < 0) 
    {
        perror("aio_write error");
        exit(-1);
    }
    ++lock_num_requests;
    sem_post(&blocking_waiter);
}

/*****************************************************************************
 函 数 名  : aio_write_handler
 功能描述  : aio异步写入函数
 输入参数  : sigval_t  sigval  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
void aio_write_handler (sigval_t sigval)
{
    size_t nbytes;
    handler_context* hctx = (handler_context*)sigval.sival_ptr;
    if (aio_error(hctx->m_aiocb)) 
    {
        perror("write aio error");
        exit(-1);
    }
    nbytes = aio_return(hctx->m_aiocb);
    sem_post(&blocking_waiter);
  //free(hctx->m_aiocb->aio_buf);
}

/*****************************************************************************
 函 数 名  : copy_regular
 功能描述  : aio异步文件复制
 输入参数  : const char* src_file  
             const char* dst_file  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月11日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
int copy_regular (const char* src_file, const char* dst_file)
{
    
    //源文件句柄和目标文件句柄
    int src_fd;
    int dst_fd;

    //内存页数和块内存指针
    uint64_t num_pages;
    void * buffer_block;
    
    // get the page_size for the system
    page_size = getpagesize();
    
    struct stat stat_buf, stat_dst;
    // stat the source file
    if (stat(src_file, &stat_buf) == -1 ) 
    {
        perror("source file stat error");
        exit(-1);
    }
    // if its a directory, create and exit
    //    if (S_ISDIR(stat_buf.st_mode)) 
    //    {
    //        if (mkdir(dst_file, S_IRWXU | S_IRWXG))
    //        {
    //            perror("mkdir error");
    //            exit(-1);
    //        }
    //        return 0;
    //    }
    // open the source file for reading RDONLY
    if ((src_fd = open(src_file, O_RDONLY)) < 0) 
    {
        perror("source file open error");
        exit(-1);
    }
    // open the destination file for writing,没有就创建,并附加上权限
    if ((dst_fd = open(dst_file, O_WRONLY| O_CREAT, stat_buf.st_mode)) < 0) 
    {
        perror("destination file open error");
        exit(-1);
    }
    
    if (fstat(dst_fd, &stat_dst) == -1) 
    {
        perror("fstat destination error");
        exit(-1);
    }
    
    // check if input and output are the same,输入输出是否相同
    if (stat_buf.st_dev  == stat_dst.st_dev && \
        stat_buf.st_ino == stat_dst.st_ino)
        return 0;
  
    // TODO tell the kernel that we will need the input file
    posix_fadvise(src_fd, 0, stat_buf.st_size, POSIX_FADV_WILLNEED);
  
    // more efficient space allocation via fallocate for dst file
    if (fallocate(dst_fd, 0, 0, stat_buf.st_size) == -1) 
        perror("destination file fallocate");
  
    // decide the number of pages in the input file and malloc a buffer accordingly
    num_pages = stat_buf.st_size / page_size + 1;
    buffer_size = page_size; 
    //(num_pages < BUF_MAX) ? (num_pages * page_size) : (BUF_MAX * page_size);

    // now start sending aio read requests
    size_t i;
    for (i = 0; i < stat_buf.st_size; i += buffer_size)
    {
        //posix_fadvise(src_fd, i, buffer_size, POSIX_FADV_SEQUENTIAL);
        buffer_block = (void *)malloc(buffer_size);
        if (errno == ENOMEM) 
        {
            perror("malloc for buffer error");
            exit(-1);
        }  
        // init aiocb struct
        struct aiocb* r_aiocb = (struct aiocb*)malloc(sizeof(struct aiocb));
        handler_context* r_context = (handler_context *) malloc(sizeof(handler_context));
        bzero ((char *)r_context, sizeof(handler_context));
        bzero ((char *)r_aiocb, sizeof(struct aiocb));
        
        // context to be passed to handler
        r_context->m_aiocb = r_aiocb;
        r_context->m_offset = i;
        r_context->m_file_size = stat_buf.st_size;
        r_context->m_src_fd = src_fd;
        r_context->m_dst_fd = dst_fd;

        // basic setup
        r_aiocb->aio_fildes = src_fd;
        r_aiocb->aio_nbytes = buffer_size;
        r_aiocb->aio_offset = i;
        r_aiocb->aio_buf = buffer_block;

        // thread callback
        r_aiocb->aio_sigevent.sigev_notify = SIGEV_THREAD;
        r_aiocb->aio_sigevent.sigev_notify_function = aio_read_handler;
        r_aiocb->aio_sigevent.sigev_notify_attributes = NULL;
        r_aiocb->aio_sigevent.sigev_value.sival_ptr = (void *)r_context;

        if (aio_read(r_aiocb) < 0) 
        {
            perror("aio_read error");
            exit(-1);
        }
        ++lock_num_requests;
    } 
    return 0;
}

/*****************************************************************************
 函 数 名  : main
 功能描述  : 主函数
 输入参数  : int argc       
             char * argv[]  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        printf("usage : %s <source> <destination>\n.", argv[0]);
        return 0;
    }
    //时间对象
    struct timespec tv1, tv2;
    lock_num_requests = 0;
    //文件开始复制前的时间
    clock_gettime(CLOCK_MONOTONIC, &tv1);
    //初始化信号量
    sem_init(&blocking_waiter, 0, 0);
    uint64_t i;
    //格式化文件路径
    src = argv[1];
    dst = argv[2];
    
    //获取源文件的属性
    struct stat src_stat, dst_stat;
    if (stat(src, &src_stat) == -1)
    {
        perror("source file stat error");
        exit(-1);
    }
    
    //获得目标文件的属性
    if (stat(dst, &dst_stat) == -1) 
    {
        // if error, must be because of a no entry
        if (errno != ENOENT)
        {
            perror("destination file stat error");
            exit(-1);
        }
        /*
            如果源文件是个目录
        */
        if (S_ISDIR(src_stat.st_mode)) 
        {
            int iRet = is_exist_dir(dst);
            if ( iRet == 1 )
            {
                printf("the dest alread exist the same directory.\n");
                traverse_dir_copy(src);
            }
            else
            {
                //S_IRWXU 00700权限，代表该文件所有者拥有读，
                //S_IRWXG 00070权限，代表该文件用户组拥有读，
                //创建一个目录, 所有者和所有组有读
                if(mkdir(dst, S_IRWXU | S_IRWXG) == -1)
                {
                    perror("destination mkdir failed");
                    exit(-1);
                }
                /*遍历并拷贝*/
                traverse_dir_copy(src);
            } 
        }
        else  
            copy_regular(src, dst);         //是个文件
    }
    
    for (i = 0; i < lock_num_requests; ++i) 
    {
        sem_wait(&blocking_waiter);
    }
    
    sem_destroy(&blocking_waiter);
    clock_gettime(CLOCK_MONOTONIC, &tv2);
    
    uint64_t tv = (tv2.tv_sec - tv1.tv_sec) * 1000000000+ tv2.tv_nsec -tv1.tv_nsec;
    printf("completion time = %ld.%06ld s\n", tv / 1000000000, tv % 1000000000);
    return 0;
}

/*****************************************************************************
 函 数 名  : Find_Last_dir_Path
 功能描述  : 查找原路径中的最后一个目录
 输入参数  : char * path  
 输出参数  : 无
 返 回 值  : char*
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
char* Find_Last_dir_Path(char * path)
{
	char *temp = (char*)malloc(strlen(path)+1);
	strcpy(temp,path);
	int i = 0;
	for(i = strlen(path) - 2; ;i--)
	{
		if(temp[i] == '/')
			break;
	}
	temp[i]=0;
	char *result = (char*)malloc(strlen(&temp[i + 1])+1);
	strcpy(result,&temp[i + 1]);
    result[strlen(result) -1] = 0; 
	free(temp);
	return result;
}

/*****************************************************************************
 函 数 名  : is_exist_dir
 功能描述  : 判断该路径含有的目录是否存在
 输入参数  : char* _dir  
 输出参数  : 无
 返 回 值  : int
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
int is_exist_dir(char* _dir)
{    
    DIR * dir = NULL;    
    dir = opendir(_dir);    
    if(dir == NULL)   
    {        
        closedir(dir); 
        return 0;    
    }    
    else    
    {        
        closedir(dir);       
        return 1;    
    }
}

/*****************************************************************************
 函 数 名  : traverse_dir_copy
 功能描述  : 递归遍历目录并复制
 输入参数  : char * pdir  
 输出参数  : 无
 返 回 值  : void
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年7月23日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
void traverse_dir_copy(char * pdir)
{
    //打开指定目录,并取得一个目录流指针
	DIR * p = opendir(pdir);
    //如果错误,直接返回
	if ( NULL == p )
	{
	   perror("opendir is error");
       return;
	}
    //定一个目录结构体对象
	struct dirent *q;
    //定一个文件属性结构体对象
	struct stat s;

    //定义起始目录路径存放变量
	char my_path[MAX_PATH];
    //定义目的目录路径存放变量
	char des_path[MAX_PATH];
    //定义临时目录路径存放变量
	char tmp_path[MAX_PATH];
	strcpy(my_path,pdir);
	strcpy(tmp_path,pdir);

    //定义起始文件路径存放变量
	char file_name[MAX_PATH];
    //定义临时文件路径存放变量
	char tmp_name[MAX_PATH];
	strcpy(file_name,pdir);
	strcpy(tmp_name,pdir);

    //目标目录
    char dst_dir[MAX_PATH];
    char dst_temp[MAX_PATH];
    strcpy(dst_dir,dst);
    strcpy(dst_temp,dst);

    //目标目录存放文件
    char dst_dir_name[MAX_PATH];
    char dst_temp_name[MAX_PATH];
    strcpy(dst_dir_name,dst);
    strcpy(dst_temp_name,dst);
    
    //读取目录
	while((q=readdir(p))!=NULL)
	{
        //遇到'.' '..'直接过滤掉
		if(q->d_name[0] == '.')
			continue;
        //这个地方可以添加其他文件属性,这里就不添加了
        //判断资源属性,为普通文件
		if(q->d_type==DT_REG)
		{
			//sprintf(des_name,"/%s",q->d_name);
			strcat(file_name,q->d_name); 
            //剔除源文件路径
            char *file_adr = (char*)malloc(MAX_PATH);
            split_str(file_name,src,&file_adr); 
            strcat(dst_dir_name,file_adr);
            free(file_adr);
            //有文件就复制
    		copy_regular(file_name, dst_dir_name);
            //还原原始的路径
			strcpy(file_name,tmp_name);
            strcpy(dst_dir_name,dst_temp_name);
            
		}
		else if ( q->d_type==DT_DIR)    //判断资源属性,为目录
		{
			sprintf(des_path,"%s/",q->d_name);
			strcat(my_path,des_path);
            char *file_adr = (char*)malloc(MAX_PATH);
            //char * file_adr = split_str(my_path,pdir);
            split_str(my_path,pdir,&file_adr);
            strcat(dst_dir,file_adr);
            free(file_adr);
            //sprintf(dst_path,"/%s",file_adr);
            //递归调用自己,继续遍历子目录
            if(mkdir(dst_dir, S_IRWXU | S_IRWXG) == -1)
            {
                perror("destination mkdir failed");
                exit(-1);
            }

            memset(des_path,0,sizeof(MAX_PATH));
            
			traverse_dir_copy(my_path);
			strcpy(my_path,tmp_path);
            strcpy(dst_dir,dst_temp);
		}
	}
	puts("\n");
	closedir(p); 
}

/*****************************************************************************
 函 数 名  : split_str
 功能描述  : 剥离源路径后剩下的文件和目录名就是需要复制新建的
 输入参数  : char *path      
             char *src_path 
             char **result
 输出参数  : 无
 返 回 值  : char *
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2017年8月12日
    作    者   : xiaofeng
    修改内容   : 新生成函数

*****************************************************************************/
char * split_str(char *path,char *src_path,char **result)
{
	int i = 0;
	char *temp = (char*)malloc(strlen(path)+1);
	strcpy(temp,path);
	for(i = 0 ;;i++)
	{
		if(temp[i] != src_path[i])
			break;
	}
	temp[i - 1] = 0;
	strcpy(*result,&temp[i]);
	free(temp);
	return *result;
}