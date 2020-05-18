/**
 * ��1500�д����0��ʼʵ��TCP/IPЭ��ջ+WEB������
 *
 * ��Դ��ּ������򵥡����׶��ķ�ʽ��������ٵ��˽�TCP/IP�Լ�HTTP����ԭ������Ҫ����֪ʶ�㡣
 * ���д��뾭�����ļ���ƣ�����ʹ���κθ��ӵ����ݽṹ���㷨������ʵ�������޹ؽ�Ҫ��ϸ�ڡ�
 *
 * ��Դ�����׸������Ƶ�̳̣�����ṩ���أ������������ַ������档
 * ��Ƶ�е�PPT���ṩ���أ���������ѧϰָ�ϣ�������������ַ��
 *
 * ���ߣ�����ͭ
 * ��ַ: http://01ketang.cc/tcpip
 * QQȺ��524699753����Ⱥʱ��ע����tcpip��������ṩ���ڸ�Դ���֧�ֺ�������
 * ΢�Ź��ںţ������� 01�γ�
 *
 * ��Ȩ������Դ�����ѧϰ�ο�������������ҵ��Ʒ������֤�ɿ��ԡ����ο�������������ǰ����ϵ���ߡ�
 * ע��
 * 1.Դ�벻�������У��ð汾���ܷ����°档�����ȡ���°棬�����������ַ��ȡ���°汾�Ĵ���
 * 2.1500�д���ָδ����ע�͵Ĵ��롣
 */
#ifndef XSERVER_HTTP_H
#define XSERVER_HTTP_H

#include "xnet_tiny.h"

#if defined(__APPLE__)      // ����ʵ������޸�
#define XHTTP_DOC_DIR               "/Users/mac/work/git/xnet-tiny/htdocs"  // html�ĵ����ڵ�Ŀ¼
#else
#define XHTTP_DOC_DIR               "d:/tiny_net"  // html�ĵ����ڵ�Ŀ¼
#endif

xnet_err_t xserver_http_create(uint16_t port);

#endif // XSERVER_HTTP_H