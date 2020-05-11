/**
 * 本源码配套的课程为 - 自己动手写TCIP/IP协议栈 源码
 * 作者：李述铜
 * 课程网址：http://01ketang.cc
 * 版权声明：本源码非开源，二次开发，或其它商用前请联系作者。
 * 注：源码不断升级中，该版本可能非最新版。如需获取最新版，请联系作者。
 * 参考网址：
 * https://nmap.org/npcap/guide/wpcap/pcap.html
 * https://nmap.org/npcap/guide/
 */
#include <memory.h>
#include "pcap_device.h"

#if defined(WIN32) || defined(WIN64)

#include <winsock.h>
#include <tchar.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")  // 加载win32的网络库

// 加载pcap的lib，根据32位或64位平台来加
#ifdef __i386
#pragma comment(lib, "..\\lib\\npcap\\Lib\\Packet.lib")  
#pragma comment(lib, "..\\lib\\npcap\\Lib\\wpcap.lib") 
#else 
#pragma comment(lib, "..\\lib\\npcap\\Lib\\x64\\Packet.lib")  
#pragma comment(lib, "..\\lib\\npcap\\Lib\\x64\\wpcap.lib") 
#endif

#define IRQ_TBL_SIZE        10
struct {
    pcap_t* pcap;
    irq_handler_t handler;
    void* arg;
}irq_table[IRQ_TBL_SIZE];

 /**
  * 设置pcap的中断处理函数
  */
void pcap_set_irq_handler(pcap_t* pcap, irq_handler_t handler, void * arg) {
    for (int i = 0; i < IRQ_TBL_SIZE; i++) {
        if (irq_table[i].pcap == (pcap_t*)0) {
            irq_table[i].pcap = pcap;
            irq_table[i].handler = handler;
            irq_table[i].arg = arg;
            break;
        }
    }
}

/**
 * 相当于接收中断处理函数
 * 只不过，数据已经读出来了，存储在header和pkt_data里面
 */
void packet_handler(u_char* param, const struct pcap_pkthdr* header, const u_char* pkt_data) {
    pcap_t* pcap = (pcap_t*)param;

    for (int i = 0; i < IRQ_TBL_SIZE; i++) {
        if (irq_table[i].pcap == pcap) {
            irq_table[i].handler(irq_table[i].arg, 1, pkt_data, header->len);
            break;
        }
    }
}

/**
 * 模拟硬件读取的线程
 */
void pcap_read_thread(void* arg) {
    pcap_t* pcap = (pcap_t*)arg;

    // 硬件不断地在检测是否收到数据包，收到后会通过中断处理函数通知应用程序
    // packet_handler在这里相当于接收中断处理程序
    while (1) {
        pcap_dispatch(pcap, -1, packet_handler, (u_char*)pcap);

#if 0
        HANDLE timer;
        LARGE_INTEGER interval;
        interval.QuadPart = -(1);
        
        // 简单释放下CPU，避免占用太高卡
        timer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (timer == (HANDLE)0) {
            return;
        }

        SetWaitableTimer(timer, &interval, 0, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
#endif
    }
}

/**
 * 调整npcap的搜索路径：默认安装在系统的dll路径\npcap目录下
 * 设置该路径，以避免使用其它已经安装的winpcap版本的dll
 * 注意：要先安装npcap软件包
 */
static int load_pcap_lib () {
    static int dll_loaded = 0;
    _TCHAR  npcap_dir[512];
    int size;

    if (dll_loaded) {
        return 0;
    }

    for (int i = 0; i < IRQ_TBL_SIZE; i++) {
        irq_table[i].pcap = (pcap_t*)0;
        irq_table[i].handler = (irq_handler_t)0;
    }

    size = GetSystemDirectory(npcap_dir, 480);
    if (!size) {
        fprintf(stderr, "Error in GetSystemDirectory: %x", GetLastError());
        return -1;
    }

    _tcscat_s(npcap_dir, 512, _T("\\Npcap"));
    if (SetDllDirectory(npcap_dir) == 0) {
        fprintf(stderr, "Error in SetDllDirectory: %x", GetLastError());
        return -1;
    }

    dll_loaded = 1;
    return 0;
}

#else   // Mac或者linux

#include <netinet/in.h>
#include <arpa/inet.h>

static int load_pcap_lib() {
    return 0;
}

#endif

/**
 * 找到指定IP地址的网卡名
 * @param ip 物理网卡或者由虚拟软件生成的虚拟刚卡, 字符串形式，如"192.168.1.1"
 * @param name_buf 找到的对应网卡名称
 */
static int pcap_find_device(const char* ip, char* name_buf) {
    char err_buf[PCAP_ERRBUF_SIZE];
    pcap_if_t* pcap_if_list = NULL;
    struct in_addr dest_ip;
    pcap_if_t* item;

    inet_pton(AF_INET, ip, &dest_ip);

    int err = pcap_findalldevs(&pcap_if_list, err_buf);
    if (err < 0) {
        pcap_freealldevs(pcap_if_list);
        return -1;
    }

    for (item = pcap_if_list; item != NULL; item = item->next) {
        if (item->addresses == NULL) {
            continue;
        }

        for (struct pcap_addr* pcap_addr = item->addresses; pcap_addr != NULL; pcap_addr = pcap_addr->next) {
            struct sockaddr_in* curr_addr;
            struct sockaddr* sock_addr = pcap_addr->addr;

            if (sock_addr->sa_family != AF_INET) {
                continue;
            }

            curr_addr = ((struct sockaddr_in*)sock_addr);
            if (curr_addr->sin_addr.s_addr == dest_ip.s_addr) {
                strcpy(name_buf, item->name);
                pcap_freealldevs(pcap_if_list);
                return 0;
            }
        }
    }

    pcap_freealldevs(pcap_if_list);
    return -1;
}

/*
 * 显示所有的网络接口列表
 */
static int pcap_show_list(void) {
    char err_buf[PCAP_ERRBUF_SIZE];
    pcap_if_t* pcapif_list = NULL;
    int count = 0;

    // 查找所有的网络接口
    int err = pcap_findalldevs(&pcapif_list, err_buf);
    if (err < 0) {
        fprintf(stderr, "pcap_show_list: find all net list failed:%s\n", err_buf);
        pcap_freealldevs(pcapif_list);
        return -1;
    }

    printf("pcap_show_list: card list\n");

    // 遍历所有的可用接口，输出其信息
    for (pcap_if_t* item = pcapif_list; item != NULL; item = item->next) {
        if (item->addresses == NULL) {
            continue;
        }

        for (struct pcap_addr* pcap_addr = item->addresses; pcap_addr != NULL; pcap_addr = pcap_addr->next) {
            char str[INET_ADDRSTRLEN];
            struct sockaddr_in* ip_addr;

            struct sockaddr* sockaddr = pcap_addr->addr;
            if (sockaddr->sa_family != AF_INET) {
                continue;
            }

            ip_addr = (struct sockaddr_in*)sockaddr;
            printf("card %d: IP:%s name: %s, \n\n",
                count++,
                item->description == NULL ? "" : item->description,
                inet_ntop(AF_INET, &ip_addr->sin_addr, str, sizeof(str))
            );
            break;
        }
    }

    pcap_freealldevs(pcapif_list);

    if ((pcapif_list == NULL) || (count == 0)) {
        fprintf(stderr, "pcap_show_list: no available card!\n");
        return -1;
    }

    return 0;
}

/**
 * 打开pcap设备接口
 * @param ip 打开网卡的指定ip
 * @param 给网卡设置mac
 */
pcap_t* pcap_device_open(const char* ip, const uint8_t * mac_addr, uint8_t poll_mode) {
    char err_buf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    bpf_u_int32 mask;
    bpf_u_int32 net;
    char filter_exp[256];
    char name_buf[256];
    pcap_t* pcap;

    if (load_pcap_lib() < 0) {
        fprintf(stderr, "pcap_open: load pcap dll failed! install it first\n");
        return (pcap_t*)0;
    }

    if (pcap_find_device(ip, name_buf) < 0) {
        fprintf(stderr, "pcap_open: no net card has ip: %s, use the following:\n", ip);
        pcap_show_list();
        return (pcap_t*)0;
    }

    if (pcap_lookupnet(name_buf, &net, &mask, err_buf) == -1) {
        printf("pcap_open: can't find use net card: %s\n", name_buf);
        net = 0;
        mask = 0;
    }

    pcap = pcap_create(name_buf, err_buf);
    if (pcap == NULL) {
        fprintf(stderr, "pcap_open: create pcap failed %s\n net card name: %s\n", err_buf, name_buf);
        fprintf(stderr, "Use the following:\n");
        pcap_show_list();
        return (pcap_t*)0;
    }

    // 获取整个包长，65535值足够
    if (pcap_set_snaplen(pcap, 65536) != 0) {
        fprintf(stderr, "pcap_open: set snaplen failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 混杂模式,接收所有地址的包
    if (pcap_set_promisc(pcap, 0) != 0) {
        fprintf(stderr, "pcap_open: set promisc failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 读超时时间
    if (pcap_set_timeout(pcap, 1) != 0) {
        fprintf(stderr, "pcap_open: set timeout failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 最后启用pcap
    if (pcap_activate(pcap) != 0) {
        fprintf(stderr, "pcap_open: active pcap failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 只捕获输入，不要捕获自己发出去的
    // 注：win平台似乎不支持这个选项
    if (pcap_setdirection(pcap, PCAP_D_IN) != 0) {
        fprintf(stderr, "pcap_open: set direction failed: %s\n", pcap_geterr(pcap));
    }

    // 非阻塞模式读取，程序中使用查询的方式读
    if (pcap_setnonblock(pcap, 1, err_buf) != 0) {
        fprintf(stderr, "pcap_open: set none block failed: %s\n", pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    // 只捕获发往本接口与广播的数据帧。相当于只处理发往这张网卡的包
    sprintf(filter_exp,
        "(ether dst %02x:%02x:%02x:%02x:%02x:%02x or ether broadcast) and (not ether src %02x:%02x:%02x:%02x:%02x:%02x)",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    if (pcap_compile(pcap, &fp, filter_exp, 0, net) == -1) {
        printf("pcap_open: couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(pcap));
        return (pcap_t*)0;
    }
    if (pcap_setfilter(pcap, &fp) == -1) {
        printf("pcap_open: couldn't install filter %s: %s\n", filter_exp, pcap_geterr(pcap));
        return (pcap_t*)0;
    }

    if (!poll_mode) {
#if defined(_WIN32) || defined(_WIN64)        // windows
        // 创建一个线程，用于模拟硬件中断来收发数据包
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pcap_read_thread, pcap, 0, NULL);
#endif
    }
    return pcap;
}

/**
 * 关闭Pcapif接口
 */
void pcap_device_close(pcap_t* pcap) {
    if (pcap == (pcap_t *)0) {
        fprintf(stderr, "pcap = 0");
        pcap_show_list();
        return;
    }
    pcap_close(pcap);
}

/**
 * 向网络接口发送数据包
 */
uint32_t pcap_device_send(pcap_t* pcap, const uint8_t* buffer, uint32_t length) {
    if (pcap_sendpacket(pcap, buffer, length) == -1) {
        fprintf(stderr, "pcap send: send packet failed!:%s\n", pcap_geterr(pcap));
        fprintf(stderr, "pcap send: pcaket size %d\n", length);
        return 0;
    }

    return 0;
}

/**
 * 从网络接口读取数据包
 */
uint32_t pcap_device_read(pcap_t* pcap, uint8_t* buffer, uint32_t length) {
    int err;
    struct pcap_pkthdr* pkthdr;
    const uint8_t* pkt_data;

    err = pcap_next_ex(pcap, &pkthdr, &pkt_data);
    if (err == 0) {
        return 0;
    } else if (err == 1) {     // 1 - 成功读取数据包, 0 - 没有数据包，其它值-出错
        memcpy(buffer, pkt_data, pkthdr->len);
        return pkthdr->len;
    }

    fprintf(stderr, "pcap_read: reading packet failed!:%s", pcap_geterr(pcap));
    return 0;
}

