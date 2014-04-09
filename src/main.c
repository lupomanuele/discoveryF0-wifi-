/*
 * ADD HEADER
 */
#include <stdio.h>
#include "target.h"
#include "spwf_wifi.h"
#include "uart_wifi.h"
#include "image.h"
//http://lwip.wikia.com/wiki/Tuning_TCP
//sudo tcpdump -i wlan0 -w tcpdump.out -s 1520 port 6768
//tcptrace -l -o3 tcpdump.out
// tcptrace -r -l -G tcpdump.out
//http://www.tcptrace.org/manual/node11_tf.html
//http://prefetch.net/blog/index.php/2006/04/17/debugging-tcp-connections-with-tcptrace/
// http://www.tcptrace.org/manual/node12_mn.html


/**
 * Setup Variables
 */
//const char *server_ip_addr = "fitnetfreedom.com"; //"192.168.1.131";
const char *server_ip_addr = "192.168.1.105";
static unsigned int server_port = 5432;
//const char *ssid = "FASTWEB-1-38229DFC50F4";
//const char *wpa_key = "6170027600";
const char *ssid = "IoT";
const char *wpa_key = "IoT1Time2Go";

/* Main Defines */
#define CHUNK_LEN 	3*1024
#define STREAM_SIZE 1024*1024*10
unsigned char pippo[CHUNK_LEN];

/*
 * Main
 */
int main(void) {
	//char m[257];
	int sock = -1;
	int err, count;
	struct wifi_spwf_conf cfg;
	cfg.baudrate = 576000; //115200; 230400 460800
	cfg.mode = sta;
	//sprintf(cfg.ssid, "IoT");
	//sprintf(cfg.wpa_key, "IoT1Time2Go");
	sprintf(cfg.ssid, ssid);
	sprintf(cfg.wpa_key, wpa_key);
	main_target_init();
	memset(pippo, 'a', CHUNK_LEN);
	memset(pippo, 'b', 20);
	memset(pippo + CHUNK_LEN - 20, 'c', 20);
	led_set(0);

	int seconds = 0;
	spwf_configure(&cfg, 1);
	while (spwf_wifi_connect_wait())
		;

	unsigned int tot = STREAM_SIZE;
	int err_stage = 0;

	/* Infinite loop */
	while (1) {
		if (sock < 0) {
			count = 0;
			uwTimingPerf = 0;
			sock = spwf_sock_connect(server_ip_addr, server_port, 't', NULL);
			led_set(0);
			continue;
		}

		if (sock >= 0 && tot > 0) {
			err = spwf_sock_write(sock, pippo,
					(tot < CHUNK_LEN) ? tot : CHUNK_LEN);
			if (err) {
				err_stage++;

				// Currently the AT+SOCKW API is not reentrant in case of error/timeout.
				// For thi is important .. if no OK token has been received..
				// se we have to check if the console is ready for next command input.
				// We can do this adding a while loop that send AT command till OK is not received.
				// This means the console is newly ready to get new commands!
				// TODO; Here we can also handle connection lost case.
				while (EVAL_WIFI_UART_send_and_test(WIFI_USART, "AT\r\n", "OK",
						2, strlen("AT\r\n"))) {
					count++;
				}
			}

			tot -= CHUNK_LEN;
		}

		if (sock >= 0 && tot <= 0) {
			err = spwf_sock_write(sock, "STREAMING DONE",
					strlen("STREAMING DONE"));
			tot = STREAM_SIZE;

			//err = 1;
			//int aa = 0;
			//while (!aa) {
			//err = spwf_sock_read(sock, line, NULL, &aa);
			//}

			spwf_sock_close(sock);
			sock = -1;

			led_set(1);
		}
	}
}
