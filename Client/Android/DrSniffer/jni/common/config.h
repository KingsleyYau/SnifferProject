/*
 * config.h
 *
 *  Created on: 2014年3月31日
 *      Author: kingsley
 */

#ifndef CONFIG_H_
#define CONFIG_H_

 /*
  * TCP_KEEPIDLE for Android in <linux/tcp.h>
  * TCP_KEEPALIVE for iOS in <netinet/tcp.h>
  */
#if !defined(TCP_KEEPIDLE) && defined(TCP_KEEPALIVE)
#define TCP_KEEPIDLE TCP_KEEPALIVE
#endif

#endif /* CONFIG_H_ */
