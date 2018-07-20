#include "LoginConn.h"
#include "netlib.h"
#include "ConfigFileReader.h"
#include "version.h"
#include "HttpConn.h"
#include "ipparser.h"
#include "uchat/logger.hpp"
#include "uchat/loghelper.cpp.hpp"
constexpr char const* kN = "login_server";///< Node name
IpParser* pIpParser = NULL;
string strMsfsUrl;
string strDiscovery;//发现获取地址
void client_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		CLoginConn* pConn = new CLoginConn();
		pConn->OnConnect2(handle, LOGIN_CONN_TYPE_CLIENT);
	}
	else
	{
		log("!!!error msg: %d ", msg);
	}
}
// this callback will be replaced by imconn_callback() in OnConnect()
void msg_serv_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    Info0(__func__ << ": msg_server come in")

	if (msg == NETLIB_MSG_CONNECT)
	{
		CLoginConn* pConn = new CLoginConn();
		pConn->OnConnect2(handle, LOGIN_CONN_TYPE_MSG_SERV);
	}
	else
	{
		log("!!!error msg: %d ", msg);
	}
}


void http_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
    if (msg == NETLIB_MSG_CONNECT)
    {
        CHttpConn* pConn = new CHttpConn();
        pConn->OnConnect(handle);
    }
    else
    {
        log("!!!error msg: %d ", msg);
    }
}
constexpr char const* kDefaultConfFilename = "loginserver.conf";
int main(int argc, char* argv[])
{
    uchat::Logger::getLogger().setOutputs(uchat::Logger::Output::CoutOrCerr);
    uchat::Logger::getLogger().setLogLevel(uchat::LogLevel::Debu);
    if ((argc == 2) && (strcmp(argv[1], "-v") == 0)) {
        Note(__func__ << ": " << kN << " build " << __DATE__ << " "
            << __TIME__)
        return 0;
    }
    std::string confFilename;
    if (argc > 1) {
        confFilename = argv[1];
    } else {
        confFilename = kDefaultConfFilename;
    }
    ::signal(SIGPIPE, SIG_IGN);
    CConfigFileReader config_file(confFilename.c_str());
    // 0.0.0.0 is OK: this server for client to request a min connections msg
    // server via socket
    char* client_listen_ip = config_file.GetConfigName("ClientListenIP");
    char* str_client_port = config_file.GetConfigName("ClientPort");
    // 0.0.0.0 is OK: request min connections msg server via HTTP
    char* http_listen_ip = config_file.GetConfigName("HttpListenIP");
    char* str_http_port = config_file.GetConfigName("HttpPort");
    // 0.0.0.0 is OK
    char* msg_server_listen_ip = config_file.GetConfigName("MsgServerListenIP");
    char* str_msg_server_port = config_file.GetConfigName("MsgServerPort");
    // NOTE must be accessable URL: will send to client
    char* str_msfs_url = config_file.GetConfigName("msfs");
    char* str_discovery = config_file.GetConfigName("discovery");
    // NOTE will send to client
    char* str_update_log = config_file.GetConfigName("update_log");
    char* str_update_ver = config_file.GetConfigName("update_version");

	if (!msg_server_listen_ip || !str_msg_server_port || !http_listen_ip
        || !str_http_port || !str_msfs_url || !str_discovery) {
		log("config item missing, exit... ");
		return -1;
	}

	uint16_t client_port = atoi(str_client_port);
	uint16_t msg_server_port = atoi(str_msg_server_port);
    uint16_t http_port = atoi(str_http_port);
    strMsfsUrl = str_msfs_url;
    strDiscovery = str_discovery;
    
    
    pIpParser = new IpParser();
    
	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;
	CStrExplode client_listen_ip_list(client_listen_ip, ';');
	for (uint32_t i = 0; i < client_listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen(client_listen_ip_list.GetItem(i), client_port, client_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}

	CStrExplode msg_server_listen_ip_list(msg_server_listen_ip, ';');
	for (uint32_t i = 0; i < msg_server_listen_ip_list.GetItemCnt(); i++) {
		ret = netlib_listen(msg_server_listen_ip_list.GetItem(i), msg_server_port, msg_serv_callback, NULL);
		if (ret == NETLIB_ERROR)
			return ret;
	}
    
    CStrExplode http_listen_ip_list(http_listen_ip, ';');
    for (uint32_t i = 0; i < http_listen_ip_list.GetItemCnt(); i++) {
        ret = netlib_listen(http_listen_ip_list.GetItem(i), http_port, http_callback, NULL);
        if (ret == NETLIB_ERROR)
            return ret;
    }
    Note(__func__ << ": server started listen on: "
        << " for client " << client_listen_ip << ":" << client_port
        << " for msgserver "
        << msg_server_listen_ip << ":" << msg_server_port
        << " for http " << http_listen_ip << ":" << http_port)
	init_login_conn();
	// log("111");
    init_http_conn(str_update_ver, str_update_log);

	log("now enter the event loop...\n");
    
    writePid();

	netlib_eventloop();

	return 0;
}
