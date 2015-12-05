#SnifferProject

Autor:  Kingsley Yau

#客户端
1.支持Android客户端, 必须root才能使用</br>

#服务端
1.支持Linux服务端</br>

交互流程
Client    				Linux Server	    		 Admin</br>
      主动连接---></br>
															<---调用API</br>
			<---发送命令</br>
			返回命令---></br>
														  返回结果---></br>

2.支持的API</br>
2.1.获取在线客户端列表</br>
2.2.获取在线客户端详细信息</br>
2.3.执行客户端命令</br>
2.4.浏览客户端目录</br>
2.5.上传客户端文件到服务器</br>
2.6.下载文件到客户端</br>											  

#交互说明
1.管理者服务接口使用http协议+json</br>
2.客户端服务接口为自定义协议+json</br>
3.传输层使用tcp协议</br>
