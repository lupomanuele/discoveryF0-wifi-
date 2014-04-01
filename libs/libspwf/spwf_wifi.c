/**
 * @file spwf_wifi.c
 * @author Manuele Lupo
 * @date 9 Sep 2013
 * @brief This file contains a set of high-level APIs to deal with the SPWF module
 *
 * It is a wrapper for most common SPWF AT commands; uses dysp_usart_wifi.c for low-level functions
 * 
 * /TODO More stress tests
 */
#include "ustime.h"
//#include "dysp_plug_rev1.h"
#include "spwf_wifi.h"
#include "uart_wifi.h" //low-level API
//#include "os_utils.h"
#define WIFI_UP_PORT GPIOC
#define WIFI_UP_PIN  GPIO_Pin_1
#define ERR_OK 0
#define NULL 0

static char sock_id[2];
extern int my_strstr(char *string1, char *string2);
extern volatile char received_string[MAX_STRLEN + 1];
extern void os_Delay(__IO uint32_t nTime);
wifi_spwf_conf_t spwf_config;
static unsigned char *miniap_ssid = (unsigned char *) "earthquakeD";
static char buff[MAX_STRLEN + 1]__attribute__ ((section(".ccm")));

void split_url(char *url, char **server, char **file);

/*! \brief spwf_config
 *  \param cfg is the data structure containing all the main wifi network informations as SSID and so on.
 *  \param prog is a flag to specify if the module need to be reconfigured or not. 0 disable the module programming step.
 *  \return 0 if OK. 
 *  Configure the wifi module. The routine return the control to the caller task only once the wifi module is
 *  connected to the specified network.
 */
err_t spwf_configure(wifi_spwf_conf_t * cfg, short prog) {
	err_t err;
	char command[256];
	char token[256];
	int wifi_mode, wifi_priv;
	unsigned char *wifi_ssid;
	EVAL_WIFI_UART_init(cfg->baudrate);

	// Map Connect GPIO PB5
#ifdef TODO
	RCC_AHB1PeriphClockCmd(WIFI_UP_AHB_PERIPH, ENABLE);

	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio);
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Pin = WIFI_UP_PIN;
	GPIO_Init(GPIOB, &gpio);
	/////////////////////////////////////////////
#endif
	if (!prog) {
		return ERR_OK;
	}

	switch (cfg->mode) {
	case sta_lp:
	case sta: // configure the module in station mode
		wifi_mode = 1;
		wifi_priv = 2;
		wifi_ssid = cfg->ssid;
		break;
	case miniap:
		wifi_mode = 3;
		wifi_priv = 0;
		wifi_ssid = miniap_ssid;
		break;
	case test:
		sprintf(command, "AT&F\r\n");
		err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 60,
				strlen(command));
		if (err != ERR_OK)
			goto FINISH;
		sprintf(command, "AT+S.STS\r\n");
		err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 60,
				strlen(command));
		if (err != ERR_OK)
			goto FINISH;
		return err;
		break;
	case poweroff:
		sprintf(command, "AT+S.SCFG=wifi_mode,0\r\n");
		err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 60,
				strlen(command));
		return ERR_OK;
		break;
	} // end of  switch

	sock_id[0] = 'f';

	// Restore default settings
//	sprintf(command, "AT&F\r\n");
//	err = EVAL_WIFI_USART_send_and_test(WIFI_USART, command, "OK", 60,
//			strlen(command));
//	if (err != ERR_OK)
//		goto FINISH;
	spwf_reset();

	sprintf(command, "AT+S.SCFG=localecho1,0\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 60,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Disable Radio
	sprintf(command, "AT+S.SCFG=wifi_mode,0\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 100,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Configure SSID
	sprintf(command, "AT+S.SSIDTXT=%s\r\n", wifi_ssid);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 100,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Set Encription mode
	sprintf(command, "AT+S.SCFG=wifi_priv_mode,%d\r\n", wifi_priv);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 200,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Configure SSID LEN
	// Configure WPA Key
	//cleanup buffers!!
	memset(command, ' ', sizeof(command));
	memset(token, ' ', sizeof(token));
	//------------------------------------
	sprintf(command, "AT+S.SCFG=wifi_wpa_psk_text,%s\r\n", cfg->wpa_key);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 100,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Enable DHCP
	sprintf(command, "AT+S.SCFG=ip_use_dhcp,1\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 100,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Enable Radio
	sprintf(command, "AT+S.SCFG=wifi_mode,%d\r", wifi_mode);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 100,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;

	// Save Configuration
	sprintf(command, "AT&W\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "OK", 300,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;
	// in all cases we reset the wifi module
	sprintf(command, "AT+CFUN=1\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, command, "RESET", 250,
			strlen(command));
	if (err != ERR_OK)
		goto FINISH;
	//spwf_reset();

	return ERR_OK;

	FINISH: return ERR_OK + 1;
}

/*! \brief spwf_sock_connect
 *  \param server is the server IP address or the DNS name
 *  \param port is the port number used to create the socket
 *  \param protocol 't' for TCP, 'u' for UDP
 *  \param connected is the callback pointer. For the moment is not handled. It should be required to register OnConnect callback events.
 *  \return socket id value. 
 *  Routine wich handle the creation of a TCP socket with the server specified as argument
 */
int spwf_sock_connect(char *server, short port, char protocol,
		err_t (*connected)(void *arg, err_t err)) {
	err_t err;

	if (protocol != 't' && protocol != 'u')
		goto FINISH;

	sprintf(buff, "AT+S.SOCKON=%s,%d,%c,0\r\n", server, port, protocol);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, buff, "ID:", 1000,
			strlen(buff));
	if (err != ERR_OK) {
		goto FINISH;
	}
	// Extract the socket ID received!!
	if (result_index > 0 && received_string[result_index] == 'I'
			&& received_string[result_index + 1] == 'D') {
		sock_id[0] = received_string[result_index + 4];
		sock_id[1] = received_string[result_index + 5];
	}

	// TBD : register connected callback to a timer to check periodically if incoming data are there!!
	// IMPORTANT!!!!!!!!!!!!!!!!!!!!!
	if (sock_id[1] == 'f')
		goto FINISH;

	return atoi(sock_id);

	FINISH: return -1;

}

/*! \brief wifi_disconnect
 *  \param None
 *  \return 0 ever. 
 *  For the moment no action are required here.
 */
err_t spwf_sock_close(int sockfd) {
	char *tmp = NULL;
	err_t err = ERR_OK + 1;
	tmp = (char *) malloc(64);
	if (tmp) {
		sprintf(tmp, "AT+S.SOCKC=0%d\r\n", sockfd);
		EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) tmp, "OK", 60,
				strlen(tmp));
		err = ERR_OK;
		free(tmp);
	}

	return err;
}

/*! \brief spwf_sock_write
 *  \param sockfd is the sockt id
 *  \param packet is the buffer containing the message body to be sent
 *  \param size is the size in byte of the packet buffer 
 *  \return 0 if OK. 
 *  read incoming data on socket specified in sockfd parameter
 */
err_t spwf_sock_write(int sockfd, volatile char *packet, int size) {
	char tmp[256];
	err_t err = ERR_OK + 1;

	sprintf(tmp, "AT+S.SOCKW=0%d,%d\r\n", sockfd, size);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, tmp, 0, 60, strlen(tmp));
	if (err) {
		while (EVAL_WIFI_UART_send_and_test(WIFI_USART, "AT\r\n", "OK", 2, strlen("AT\r\n")))
			printf("sock_write error hdr\n");

		return err;
	}
	os_Delay(10);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) packet, "OK", 100, size);
	if (err) {
		while (EVAL_WIFI_UART_send_and_test(WIFI_USART, "AT\r\n", "OK", 2, strlen("AT\r\n")))
			printf("sock_write error data\n");
	}

	return err;
}

/*! \brief wifi_read
 *  \param sockfd is the socket id
 *  \param packet is the buffer used to store the incoming data
 *  \param token ???
 *  \param size pointer to the integer variable to return the number of bytes received on the socket.
 *  \return 0 if OK. 
 *  read incoming data on socket specified in sockfd parameter
 */
err_t spwf_sock_read(int sockfd, void *packet, uint8_t *token, int *size) {
	err_t err = ERR_OK;
	int i;
	char *datalen = NULL;
	sprintf(buff, "AT+S.SOCKQ=0%d\r\n", sockfd);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, buff, "DATALEN:", 60,
			strlen(buff));
	if (err != ERR_OK) {
		*size = 0;
		goto FINISH;
	}

	// Extract the data len!! Add /r position
	if (result_index > 0 && received_string[result_index] == 'D'
			&& received_string[result_index + 1] == 'A'
			&& received_string[result_index + 2] == 'T') {
		uint16_t ok_index = my_strstr(((char*) received_string + result_index),
				"OK");
		if (ok_index < 0) {
			*size = 0;
			goto FINISH;
		}

		datalen = (char *) malloc(ok_index - 9 + 1);
		i = 0;
		while (i < ok_index - 9) {
			datalen[i] = received_string[result_index + 9 + i];
			i++;
		}

		datalen[i] = '\0';
	}

	if (atoi(datalen) == 0) {
		*size = 0;
		goto FINISH;
	}

	memset(buff, 0, MAX_STRLEN);
	sprintf(buff, "AT+S.SOCKR=0%d,%d\r\n", sockfd, atoi(datalen));
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, buff, "OK", 100,
			strlen(buff));
	if (err != ERR_OK) {
		*size = 0;
		goto FINISH;
	}

	// NOTE: the user will specify the expected length for the message payload size. If it is longer it means
	// Other messages have been received in the meanwhile. Thand the user have to manage this.
	i = atoi(datalen);
	if (i > MAX_STRLEN)
		i = MAX_STRLEN;

	int index = 0;
	if (*size != 0 && i >= *size && i < MAX_STRLEN) {
		if (token != NULL && ((result_index + strlen(buff) + i) < MAX_STRLEN)) {
			index = my_strstr(
					(char*) received_string + result_index + strlen(buff) - 1,
					(char*) token);

			if (index >= 0) {
				i = *size;
			} else {
				index = 0;
			}
		}

		memcpy((void *) packet,
				(void *) received_string + result_index + strlen(buff) - 1
						+ index, i);
		*size = i;
	} else if (i>0 && *size == 0) {
		memcpy((void *) packet, (void *) received_string, i);
		*size = i;
	} else
		*size = 0;

	((char*) packet)[*size] = '\0';

	FINISH:
	// Cleanup buffer
	//EVAL_WIFI_USART_cleanupBuffer2RB(USART2);

	if (datalen)
		free(datalen);
	return err;
}

/**
 *
 * @param server
 * @param port
 * @param packet
 * @param size
 * @param answer
 * @return
 */
err_t spwf_post(char *server, short port, volatile char *packet, int *size,
		char *answer) {
	int sock_id = -1;
	int ret = ERR_OK + 1;
	//Connect
	if ((sock_id = spwf_sock_connect(server, port, 't', NULL)) < 0) {
		return ret;
	}

	// Write
	if (spwf_sock_write(sock_id, packet, *size) != ERR_OK) {
		goto FINISH;
	}

	// Wait answer
	if (answer) {
		int retry = 10;
		do {
			os_Delay(100);
			*size = 4;
			spwf_sock_read(sock_id, answer, NULL, size);
		} while (retry-- > 0 && *size == 0);
	}

	FINISH:

	spwf_sock_close(sock_id);

	return ret;
}

/**
 *
 * @param url
 * @param result
 * @param size
 * @return
 */
err_t spwf_httpget(char *url, char *result, int *size) {
	err_t err = ERR_OK;
	int nochars = 0;
	char *server, *file;
	split_url(url, &server, &file);
	sprintf(buff, "AT+S.HTTPGET=%s,%s\r\n", server, file);
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff, "OK", 400,
			strlen(buff));
	nochars = strlen((char *) received_string);
	if (nochars > 0) {
		memcpy(result, (char *) received_string, nochars);
		result[nochars + 1] = '\0';
		return err;
	} else
		return ERR_OK + 1;
}

/**
 * Routine to perform firmware update over the air
 * @param url
 * @return 0 if OK
 */
err_t spwf_fota(char *url) {
	err_t err = ERR_OK;
	char *server, *file;

	split_url(url, &server, &file);
	sprintf(buff, "AT+S.SCFG=nv_manuf,ST\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff, "OK", 60,
			strlen(buff));
	sprintf(buff, "AT+S.SCFG=nv_model,SPWF01Sx.1y\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff, "OK", 60,
			strlen(buff));
	sprintf(buff, "AT+S.NVW=8675309\r\n");
	err = EVAL_WIFI_USART_send_and_test(WIFI_USART, (char*) buff, "OK", 60,
			strlen(buff));

	//sprintf(buff, "AT+S.FWUPDATE=192.168.1.100,/fw/SPWF01S-140128.ota\r\n", server_addr, server_file_path);
	sprintf(buff, "AT+S.FWUPDATE=%s,%s\r\n", server, file);
	do {
		err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff,
				"Complete", 60000, strlen(buff));
		os_Delay(3000); // Wait 3 seconds before to retry or continue!!
	} while (err != ERR_OK);

	//TODO all error checking goes here
	sprintf(buff, "AT+S.CFUN=1\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff, "RESET", 60,
			strlen(buff));

	while (spwf_wifi_connect_wait())
		;

	sprintf(buff, "AT&F\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, buff, "OK", 60,
			strlen(buff));

	return err;
}

/**
 * Restore WiFi default settings
 * @return
 */
err_t spwf_dafault_restore(void) {
	err_t err = ERR_OK;
	sprintf(buff, "AT&F\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, buff, "OK", 60,
			strlen(buff));
	return err;
}
/**
 * Routine to perform file-system update over the air
 * @param url
 * @return 0 if OK
 */
err_t spwf_fsupdate(char *url) {
	err_t err = ERR_OK + 1;
	char *server, *file;
	split_url(url, &server, &file);
	//sprintf(buff, "AT+S.HTTPDFSUPDATE=www.dsui.it,/st/fw/fs.img\r\n");
	sprintf(buff, "AT+S.HTTPDFSUPDATE=%s,%s\r\n", server, file);
	//sprintf(buff, "AT+S.HTTPDFSUPDATE=www.dsui.it,/st/fw/ext_fs_clean.img\r\n");
	do {
		err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff,
				"Complete", 8000, strlen(buff));
		os_Delay(3000);
	} while (err != ERR_OK);
	sprintf(buff, "AT+CFUN=1\r\n");
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, (char*) buff, "RESET", 100,
			strlen(buff));
	return err;
}

/**
 * Modue Software Reset
 * @return
 */
err_t spwf_reset() {
	err_t err = ERR_OK;
	// keep wifi module reset
#if 0
	dysp_wifi_reset(ON);
	os_Delay(10);
	dysp_wifi_reset(OFF);
	os_Delay(10);
#endif
	return err;
}

/* System Error Manage */

/**
 * NONE
 */
void spwf_SystemError() {
	os_Delay(30);
	NVIC_SystemReset();
}

/**
 *
 * @return 0 if OK. 1 on Error
 */
uint8_t spwf_wifi_connect_wait() {
#if 0
	// Wait WIFI module is connected & And in case alert error!!!!
	static uint8_t wifiStatus = 0xff;
	uint8_t retry = 200;

	uint8_t wifiUpPinStatus = GPIO_ReadInputDataBit(WIFI_UP_PORT, WIFI_UP_PIN);

	// Check initial GPIO status
	if (wifiStatus != 0xff && wifiStatus != wifiUpPinStatus) {
		wifiStatus = 0xff;
		return 1; // Connection lost!!
	} else if (wifiStatus == wifiUpPinStatus) {
		return 0;
	}

	// Wait to be connected
	while (wifiUpPinStatus == GPIO_ReadInputDataBit(WIFI_UP_PORT, WIFI_UP_PIN)) {
		os_Delay(100);
		//if (retry-- == 0)
		//	break;

	}

	// Save the GPIO state
	wifiStatus = GPIO_ReadInputDataBit(WIFI_UP_PORT, WIFI_UP_PIN);

	//TODO : handle here error connection condition
	if (retry == 0) {
		//spwf_SystemError(); // Reset full platform
		return 1;
	}
#endif
	return GPIO_ReadInputDataBit(WIFI_UP_PORT, WIFI_UP_PIN);
}

/**
 *
 * @return enum value accordingly to wifi_mode_t enum
 */
wifi_mode_t spwf_mode_check() {
	err_t err = ERR_OK + 1;
	wifi_mode_t ret = 0;
	err = EVAL_WIFI_UART_send_and_test(WIFI_USART, "AT+S.GCFG=wifi_mode\r",
			"OK", 80, 20);
	if (err == ERR_OK) {
		if (strstr(((char*) received_string), "#  wifi_mode = 1") != NULL) {
			ret = miniap;
		}
		if (strstr(((char*) received_string), "#  wifi_mode = 3") != NULL) {
			ret = sta;
		}
	}

	return ret;
}

/**
 * splits a URL according to bloody AT syntax
 * @param [in] *url: string to be split either with http:// prefix or without
 * @param [out] **server the server part, pointer to statically allocated string is returned
 * @param [out] **file the file part, as a pointer in the the original url string
 */
void split_url(char *url, char **server, char **file) {
	static char server_store[32];
	char *tmp1, *tmp2;

	tmp1 = strstr(url, "://"); //search for protocol separator, if any
	if (tmp1) {
		tmp2 = strstr(tmp1 + 3, "/");
		memcpy(server_store, tmp1 + 3, tmp2 - tmp1 - 3);
		server_store[tmp2 - tmp1 - 2] = '\0';
	} else {
		tmp2 = strstr(url, "/");
		memcpy(server_store, url, tmp2 - url);
		server_store[tmp2 - url] = '\0';
	}
	*file = tmp2;
	*server = server_store;
}

/**
 *
 * @param filename
 * @return 0
 */
err_t spwf_ram_file_delete(char *filename) {
	char msg[64];
	sprintf(msg, "AT+S.FSD=/%s\r\n", filename);
	// We don't want to manage error case !!!!
	EVAL_WIFI_USART_send_and_test(WIFI_USART, msg, NULL, 2, strlen(msg));

	return ERR_OK;
}

/**
 *
 * @param filename
 * @param maxlen max file size allowed
 * @return 0 if OK. 1 on Error
 */
err_t spwf_ram_file_create(char *filename, int maxlen) {
	char msg[64];
	sprintf(msg, "AT+S.FSC=/%s,%d\r\n", filename, maxlen);
	if (EVAL_WIFI_UART_send_and_test(WIFI_USART, msg, "OK", 2, strlen(msg)))
		return ERR_OK + 1;

	return ERR_OK;
}

/**
 *
 * @param filename
 * @param data
 * @return 0 if OK. 1 on Error
 */
err_t spwf_ram_file_append(char *filename, char *data) {
	char msg[64];
	sprintf(msg, "AT+S.FSA=/%s,%d\r", filename, strlen(data));
	if (EVAL_WIFI_UART_send_and_test(WIFI_USART, msg, NULL, 5, strlen(msg)))
		return ERR_OK + 1;
	if (EVAL_WIFI_UART_send_and_test(WIFI_USART, data, "OK", 10, strlen(data)))
		return ERR_OK + 1;

	return ERR_OK;
}

/**
 *
 * @param filename
 * @param data buffer
 * @return 0 if OK. 1 on Error
 */
err_t spwf_ram_file_overwrite(char *filename, char *data) {
	// Delete File

	if (spwf_ram_file_delete(filename))
		return ERR_OK + 1;
	// Create a new file and set maxlen
	if (spwf_ram_file_create(filename, strlen(data) + 1))
		return ERR_OK + 1;
	// Append data buffer
	if (spwf_ram_file_append(filename, data))
		return ERR_OK + 1;

	return ERR_OK;
}
