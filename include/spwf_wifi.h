/**
 * @file spwf_wifi.h
 * @author Manuele Lupo
 * @date 9 Sep 2013
 */

#ifndef __WIFI_H__
#define __WIFI_H__

#define MAX_SSID_LEN	32
#define MAX_WPA_LEN		64
#define err_t int
#define u32t unsigned int
/*
 * Callback for accepted connection
 */
typedef err_t (*wifi_accept_fn)(void *arg, err_t err);

typedef enum {
	poweroff = 0, miniap = 1, sta = 2, sta_lp = 3, //activate low power mode
	test = 4
} wifi_mode_t;

/** @struct wifi_spwf_conf_t
 *  @brief contains all the main WiFi settings information
 */
typedef struct wifi_spwf_conf {
	u32t baudrate;		///< wifi baudrate
	//unsigned char *ssid;		///< SSID string
	//unsigned char *wpa_key;	///< WPA Key string
	unsigned char ssid[MAX_SSID_LEN];
	unsigned char wpa_key[MAX_WPA_LEN];
	wifi_mode_t mode;
	unsigned char *sock_id;	///< temporary buffer to store latest socket ACK id received. TODO Must be removed!!
} wifi_spwf_conf_t;
/*
 * Connect Routine
 */
err_t spwf_configure(wifi_spwf_conf_t * cfg, short prog);
wifi_mode_t spwf_mode_check();
int spwf_sock_connect(char *server, short port, char protocol,
		err_t (*connected)(void *arg, err_t err));
err_t spwf_sock_read(int sockfd, void *packet, uint8_t *token, int *size);
err_t spwf_sock_write(int sockfd, volatile char *packet, int size);
err_t spwf_sock_close(int sockfd);
err_t spwf_post(char *server, short port, volatile char *packet, int *size,
		char *answer);
err_t spwf_httpget(char *url, char *result, int *size);
err_t spwf_fota(char *url);
err_t spwf_fsupdate(char *url);
uint8_t spwf_wifi_connect_wait();
err_t spwf_configure_uart_speed(uint32_t speed, uint8_t hwfc);
err_t spwf_reset();

/*
 * RAM File Routines
 */
err_t spwf_ram_file_delete(char *filename);
err_t spwf_ram_file_create(char *filename, int maxlen);
err_t spwf_ram_file_append(char *filename, char *data);
err_t spwf_ram_file_overwrite(char *filename, char *data);

#endif /* WIFI */
