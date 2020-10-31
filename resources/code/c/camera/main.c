/**
操作步骤：
1、打开设备/dev/videox
2. (option)取得设备的capability，看看设备具有什么功能，比如是否具有视频输入,或者音频输入输出等 VIDIOC_QUERYCAP
3. (option)查看支持的帧格式 VIDIOC_ENUM_FMT
4. 设置视频的制式和帧格式. 制式包括PAL\NTSC，帧格式包括宽度和高度等. VIDIOC_S_STD,VIDIOC_S_FMT
5. 向驱动申请帧缓冲，一般不超过5个 VIDIOC_REQBUFS
6. 向驱动查询申请到的帧缓冲 VIDIOC_QUERYBUF
7. 将申请到的帧缓冲映射到用户空间，这样就可以直接操作采集到的帧了，而不必去复制。mmap
8. 将申请到的帧缓冲全部入队列，以便存放采集到的数据 VIDIOC_QBUF
9. 开始视频的采集 VIDIOC_STREAMON
10、判断缓冲区是否有数据, 使用poll函数
11. 出队列以取得已采集数据的帧缓冲，取得原始采集数据 VIDIOC_DQBUF
12. 将缓冲重新入队列尾,这样可以循环采集 VIDIOC_QBUF
13. 停止视频的采集 VIDIOC_STREAMOFF
14. 关闭视频设备 close(fd);
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/mman.h>
#include <poll.h>
#include <strings.h>

#define DEBUG
#include <debug.h>

struct vivi_dev {
	int fd;
	int width;
	int height;
	int bufNumber;
	struct frameBuffer *frameBuf;
};

struct frameBuffer{
	unsigned int length;
	void *start;
};

#pragma pack(1) // 作用：使结构体按1字节方式对齐
typedef struct BITMAPFILEHEADER
{
	unsigned short bfType;//位图文件的类型,
	unsigned long bfSize;//位图文件的大小，以字节为单位
	unsigned short bfReserved1;//位图文件保留字，必须为0
	unsigned short bfReserved2;//同上
	unsigned long bfOffBits;//位图阵列的起始位置，以相对于位图文件   或者说是头的偏移量表示，以字节为单位
} BITMAPFILEHEADER;
#pragma pack()


typedef struct BITMAPINFOHEADER//位图信息头类型的数据结构，用于说明位图的尺寸
{
	unsigned long biSize;//位图信息头的长度，以字节为单位
	unsigned long biWidth;//位图的宽度，以像素为单位
	unsigned long biHeight;//位图的高度，以像素为单位
	unsigned short biPlanes;//目标设备的级别,必须为1
	unsigned short biBitCount;//每个像素所需的位数，必须是1(单色),4(16色),8(256色)或24(2^24色)之一
	unsigned long biCompression;//位图的压缩类型，必须是0-不压缩，1-BI_RLE8压缩类型或2-BI_RLE4压缩类型之一
	unsigned long biSizeImage;//位图大小，以字节为单位
	unsigned long biXPelsPerMeter;//位图目标设备水平分辨率，以每米像素数为单位
	unsigned long biYPelsPerMeter;//位图目标设备垂直分辨率，以每米像素数为单位
	unsigned long biClrUsed;//位图实际使用的颜色表中的颜色变址数
	unsigned long biClrImportant;//位图显示过程中被认为重要颜色的变址数
} BITMAPINFOHEADER;

void yuv422_2_rgb(struct vivi_dev *viviDev, struct v4l2_buffer *buf, \
		unsigned char *starter, unsigned char *newBuf)
{
	unsigned char YUV[4],RGB[6];
	unsigned int i,j,k=0;
	unsigned int location = 0;

	PRI_DEBUG("starter=%p, length=0x%x\n",
		starter, (viviDev->frameBuf + buf->index)->length);

	for(i = 0; i < (viviDev->frameBuf + buf->index)->length; i+=4)
	{
		YUV[0] = starter[i];		// y
		YUV[1] = starter[i+1];		// u
		YUV[2] = starter[i+2];		// y
		YUV[3] = starter[i+3];		// v
		if(YUV[0] < 0){
			RGB[0]=0;
			RGB[1]=0;
			RGB[2]=0;
		}else{
			RGB[0] = YUV[0] + 1.772*(YUV[1]-128);		// b
			RGB[1] = YUV[0] - 0.34414*(YUV[1]-128) - 0.71414*(YUV[3]-128);		// g
			RGB[2] = YUV[0 ]+ 1.402*(YUV[3]-128);			// r
		}
		if(YUV[2] < 0)
		{
			RGB[3]=0;
			RGB[4]=0;
			RGB[5]=0;
		}else{
			RGB[3] = YUV[2] + 1.772*(YUV[1]-128);		// b
			RGB[4] = YUV[2] - 0.34414*(YUV[1]-128) - 0.71414*(YUV[3]-128);		// g
			RGB[5] = YUV[2] + 1.402*(YUV[3]-128) ;			// r
		}

		for(j = 0; j < 6; j++){
			if(RGB[j] < 0)
				RGB[j] = 0;
			if(RGB[j] > 255)
				RGB[j] = 255;
		}
		//请记住：扫描行在位图文件中是反向存储的！
		if(k%(viviDev->width*3)==0)//定位存储位置
		{
			location=(viviDev->height-k/(viviDev->width*3))*(viviDev->width*3);
		}
		bcopy(RGB, newBuf+location+(k%(viviDev->width*3)), sizeof(RGB));
		k+=6;
	}
}

void create_bmp_header(struct vivi_dev *viviDev, \
		struct BITMAPFILEHEADER *bfh, struct BITMAPINFOHEADER *bih)
{
	PRI_DEBUG("width=%d, height=%d\n",
			viviDev->width, viviDev->height);

	bfh->bfType = (unsigned short)0x4D42;
	bfh->bfSize = (unsigned long)(14 + 40 + viviDev->width * viviDev->height*3);
	bfh->bfReserved1 = 0;
	bfh->bfReserved2 = 0;
	bfh->bfOffBits= (unsigned long)(14 + 40);

	bih->biBitCount = 24;
	bih->biWidth = viviDev->width;
	bih->biHeight = viviDev->height;
	bih->biSizeImage = viviDev->width * viviDev->height * 3;
	bih->biClrImportant = 0;
	bih->biClrUsed = 0;
	bih->biCompression = 0;
	bih->biPlanes = 1;
	bih->biSize = 40; //sizeof(bih);
	bih->biXPelsPerMeter = 0x00000ec4;
	bih->biYPelsPerMeter=0x00000ec4;
}

/* 1、打开设备/dev/videox */
int openCamera(struct vivi_dev *viviDev, int id)
{
	char devicename[12];

	PRI_DEBUG("\n");

	sprintf(devicename,"/dev/video%d",id);
	viviDev->fd = open(devicename, O_RDWR | O_NONBLOCK, 0);
	if(viviDev->fd < 0) {
		PRI_ERROR("open %s fail.\n", devicename);
		return -1;
	}

	return 0;
}

/* 2. (option)取得设备的capability，看看设备具有什么功能，比如是否具有视频输入,或者音频输入输出等 VIDIOC_QUERYCAP */
void capabilityCamera(struct vivi_dev *viviDev)
{
	struct v4l2_capability cap;

	ioctl(viviDev->fd, VIDIOC_QUERYCAP, &cap);
	PRI_DEBUG("\ndriver : %s\ncard : %s\n", cap.driver, cap.card);
}

/* 3. (option)查看支持的帧格式 VIDIOC_ENUM_FMT */
void enumfmtCamera(struct vivi_dev *viviDev)
{
	struct v4l2_fmtdesc fmtdesc;

	memset(&fmtdesc, 0, sizeof(fmtdesc));
	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while((ioctl(viviDev->fd, VIDIOC_ENUM_FMT, &fmtdesc)) != -1)
	{
		PRI_DEBUG("index:%d \tdescription:%s \n", fmtdesc.index, fmtdesc.description);

		fmtdesc.index++;
	}
}

/*
4. 设置视频的制式和帧格式.
   制式包括PAL\NTSC，帧格式包括宽度和高度等. VIDIOC_S_STD,VIDIOC_S_FMT
 */
int setfmtCamera(struct vivi_dev *viviDev)
{
	int ret;
	struct v4l2_format format;

	PRI_DEBUG("\n");

	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = viviDev->width;
	format.fmt.pix.height = viviDev->height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;  // 设置为yuyv格式数据
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = ioctl(viviDev->fd, VIDIOC_S_FMT, &format);
	if(ret < 0){
		PRI_ERROR("VIDIOC_S_FMT fail\n");
		return -1;
	}

	return 0;
}

/* 5. 向驱动申请帧缓冲，一般不超过5个 VIDIOC_REQBUFS */
/* 6. 向驱动查询申请到的帧缓冲 VIDIOC_QUERYBUF */
/* 7. 将申请到的帧缓冲映射到用户空间，这样就可以直接操作采集到的帧了，而不必去复制。mmap */
int initmmap(struct vivi_dev *viviDev)
{
	struct v4l2_requestbuffers reqbuf;
	__u32 i, ret;

	PRI_DEBUG("\n");

	reqbuf.count = viviDev->bufNumber;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(viviDev->fd, VIDIOC_REQBUFS, &reqbuf);
	if(0 != ret) {
		PRI_ERROR("VIDIOC_REQBUFS fail\n");
		return -1;
	}

	viviDev->frameBuf = calloc(viviDev->bufNumber, sizeof(struct frameBuffer));

	for(i =0; i < reqbuf.count; i++) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(viviDev->fd, VIDIOC_QUERYBUF, &buf);

		(viviDev->frameBuf+i)->length = buf.length;
		(viviDev->frameBuf+i)->start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE,
			MAP_SHARED, viviDev->fd, buf.m.offset);
		if((viviDev->frameBuf+i)->start == MAP_FAILED) {
			PRI_ERROR("mmap fail.\n");
			return -1;
		}

		PRI_DEBUG("frameBuf: start=%p  length=0x%x\n",
				(viviDev->frameBuf+i)->start ,
				(viviDev->frameBuf+i)->length);
	}

	return 0;
}

/* 8. 将申请到的帧缓冲全部入队列，以便存放采集到的数据 VIDIOC_QBUF */
/* 9. 开始视频的采集 VIDIOC_STREAMON */
static int startcap(struct vivi_dev *viviDev)
{

	enum v4l2_buf_type type;
	struct v4l2_buffer buf;
	int ret, i = 0;

	PRI_DEBUG("\n");

	for(i = 0; i < viviDev->bufNumber; i++){
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		ret = ioctl(viviDev->fd, VIDIOC_QBUF, &buf);
		if(0 != ret){
			PRI_ERROR("VIDIOC_QBUF fail.\n");
			return -1;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(viviDev->fd, VIDIOC_STREAMON, &type);

	return ret;
}

/* 10、判断缓冲区是否有数据, 使用poll函数 */
/* 11. 出队列以取得已采集数据的帧缓冲，取得原始采集数据 VIDIOC_DQBUF */
/* 12. 将缓冲重新入队列尾,这样可以循环采集 VIDIOC_QBUF */
static int readfram(struct vivi_dev *viviDev)
{
	struct pollfd pollfd;
	struct v4l2_buffer buf;
	unsigned char *starter;
	unsigned char *newBuf;
	struct BITMAPFILEHEADER bfh;
	struct BITMAPINFOHEADER bih;
	int ret;

	PRI_DEBUG("\n");

	memset(&pollfd, 0, sizeof(pollfd));
	pollfd.fd = viviDev->fd;
	pollfd.events = POLLIN;

	ret = poll(&pollfd, 1, 800);
	if(ret == -1){
		PRI_ERROR("VIDIOC_QBUF fail.\n");
		return ret;
	}else if(0 == ret){
		PRI_WARN("poll time out\n");
	}

	if(pollfd.revents & POLLIN){
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(viviDev->fd, VIDIOC_DQBUF, &buf);
		if(0 != ret){
			PRI_ERROR("VIDIOC_QBUF fail.\n");
			return -1;
		}

		// RGB格式数据
		starter = (unsigned char*)(viviDev->frameBuf + buf.index)->start;
		newBuf = (unsigned char*)calloc((unsigned int)((viviDev->frameBuf + buf.index)->length*3/2), sizeof(unsigned char));
		yuv422_2_rgb(viviDev, &buf, starter, newBuf);
		create_bmp_header(viviDev, &bfh, &bih);

		FILE *file = fopen("rgb.bmp", "wb");
		fwrite(&bfh, sizeof(bfh), 1, file);
		fwrite(&bih, sizeof(bih), 1, file);
		fwrite(newBuf, 1, buf.length*3/2, file);
		fclose(file);

		ret = ioctl(viviDev->fd, VIDIOC_QBUF, &buf);
	}

	return ret;
}

/* 13. 停止视频的采集 VIDIOC_STREAMOFF */
/* 14. 关闭视频设备 close(fd); */
static void closeCamera(struct vivi_dev *viviDev)
{
	int ret=-1, i;
	enum v4l2_buf_type type;

	PRI_DEBUG("\n");

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(viviDev->fd, VIDIOC_STREAMOFF, &type);
	if(ret != 0) {
		PRI_ERROR("VIDIOC_QBUF fail.\n");
		return ;
	}

	for(i = 0; i < viviDev->bufNumber; i++) {
		munmap((viviDev->frameBuf+i)->start, (viviDev->frameBuf+i)->length);
	}
	free(viviDev->frameBuf);
	close(viviDev->fd);
}

int main(int argc, char* argv[])
{
	struct vivi_dev viviDev;

	if(argc != 2){
		PRI_INFO("usage:%s <0/1/2/...> \n",argv[0]);
		return -1;
	}
	else {
		PRI_INFO("use /dev/video%d\n", atoi(argv[1]));
	}

	openCamera(&viviDev, atoi(argv[1]));

	capabilityCamera(&viviDev);
	enumfmtCamera(&viviDev);

	viviDev.width = 640;
	viviDev.height = 480;
	setfmtCamera(&viviDev);

	viviDev.bufNumber = 3;
	initmmap(&viviDev);

	startcap(&viviDev);
	readfram(&viviDev);
	closeCamera(&viviDev);

	return 0;
}

