#include "ran.h"

string g_ran_ip_addr = RAN;
string g_trafmon_ip_addr = RAN;
string g_mme_ip_addr = MMELB;
int g_trafmon_port = 4000;
int g_mme_port = 5000;
string g_sgw_s1_ip_addr = SGWLB;
int sgw_s1_port = 7100;

RanContext::RanContext() {
	emm_state = 0; 
	ecm_state = 0; 
	imsi = 0; 
	guti = 0; 
	ip_addr = "";
	enodeb_s1ap_ue_id = 0; 
	mme_s1ap_ue_id = 0; 
	tai = 1; 
	tau_timer = 0;
	key = 0; 
	k_asme = 0; 
	ksi_asme = 7; 
	k_nas_enc = 0; 
	k_nas_int = 0; 
	nas_enc_algo = 0; 
	nas_int_algo = 0; 
	count = 1;
	bearer = 0;
	dir = 0;
	apn_in_use = 0; 
	eps_bearer_id = 0; 
	e_rab_id = 0; 
	s1_uteid_ul = 0; 
	s1_uteid_dl = 0; 
	mcc = 1; 
	mnc = 1; 
	plmn_id = g_telecom.get_plmn_id(mcc, mnc);
	msisdn = 0; 
	nw_capability = 1;
}

void RanContext::init(uint32_t arg) {
	enodeb_s1ap_ue_id = arg;
	key = arg;
	msisdn = 9000000000 + arg;
	imsi = g_telecom.get_imsi(plmn_id, msisdn);
}

RanContext::~RanContext() {

}

EpcAddrs::EpcAddrs() {
	mme_port = g_mme_port;
	sgw_s1_port = 0;
	mme_ip_addr = g_mme_ip_addr;	
	sgw_s1_ip_addr = "";
}

EpcAddrs::~EpcAddrs() {

}

void UplinkInfo::init(uint32_t arg_s1_uteid_ul, string arg_sgw_s1_ip_addr, int arg_sgw_s1_port) {
	s1_uteid_ul = arg_s1_uteid_ul;
	sgw_s1_ip_addr = arg_sgw_s1_ip_addr;
	sgw_s1_port = arg_sgw_s1_port;
}

TrafficMonitor::TrafficMonitor() {
	uplink_info.clear();
	g_sync.mux_init(uplinkinfo_mux);

}
void TrafficMonitor::initialize() {
	sgw_s1_clients.resize(client_count);
	for (int i = 0; i < client_count; i++) {
		sgw_s1_clients[i].conn(g_trafmon_ip_addr, g_sgw_s1_ip_addr, sgw_s1_port);
	}
}
int getHash(string ip,int size){

	ip = ip.substr(ip.find_last_of('.')+1, ip.size());
	int index = stoi(ip);
	index = index % size;
	return index;
}
void TrafficMonitor::handle_uplink_udata() {
	Packet pkt;
	string ip_addr;
	uint32_t s1_uteid_ul;
	string sgw_s1_ip_addr;
	int sgw_s1_port;
	int index;
	bool res;

	tun.rcv(pkt);
	ip_addr = g_nw.get_src_ip_addr(pkt);

	res = get_uplink_info(ip_addr, s1_uteid_ul, sgw_s1_ip_addr, sgw_s1_port);
	if (res == true) {
		index = getHash(ip_addr,sgw_s1_clients.size());
		pkt.prepend_gtp_hdr(1, 1, pkt.len, s1_uteid_ul);
		sgw_s1_clients[index].snd(pkt);

	}
}
void TrafficMonitor::handle_downlink_udata() {
	Packet pkt;
	struct sockaddr_in src_sock_addr;
	server.rcv(src_sock_addr, pkt);
	TRACE(cout << "trafficmonitor_handledownlinkudata:" << " " << pkt.len << endl;)
	pkt.extract_gtp_hdr();
	pkt.truncate();
	tun.snd(pkt);

}

void TrafficMonitor::update_uplink_info(string ip_addr, uint32_t s1_uteid_ul, string sgw_s1_ip_addr, int sgw_s1_port) {
	g_sync.mlock(uplinkinfo_mux);
	uplink_info[ip_addr].init(s1_uteid_ul, sgw_s1_ip_addr, sgw_s1_port);
	g_sync.munlock(uplinkinfo_mux);
}

bool TrafficMonitor::get_uplink_info(string ip_addr, uint32_t &s1_uteid_ul, string &sgw_s1_ip_addr, int &sgw_s1_port) {
	bool res = false;

	g_sync.mlock(uplinkinfo_mux);
	if (uplink_info.find(ip_addr) != uplink_info.end()) {
		res = true;
		s1_uteid_ul = uplink_info[ip_addr].s1_uteid_ul;
		sgw_s1_ip_addr = uplink_info[ip_addr].sgw_s1_ip_addr;
		sgw_s1_port = uplink_info[ip_addr].sgw_s1_port;
	}
	g_sync.munlock(uplinkinfo_mux);
	return res;
}

TrafficMonitor::~TrafficMonitor() {

}

void Ran::init(int arg) {
	ran_ctx.init(arg);
}

void Ran::conn_mme() {

	if(UE_BINDING == STATE_LESS){
		mme_client_attach_1.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
		mme_client_attach_2.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
		mme_client_attach_3.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
		mme_client_attach_4.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
		mme_client_detach.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
	}	
	else if(UE_BINDING == SESSION_BASED){
		mme_client_attach_1.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
		mme_client_detach.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
	}
	else if(UE_BINDING == UE_BASED){
		mme_client_attach_1.conn(epc_addrs.mme_ip_addr, epc_addrs.mme_port);
	}
	

}
void Ran::clean_conn_mme() {

	if(UE_BINDING == STATE_LESS){
		mme_client_attach_1.release();
		mme_client_attach_2.release();
		mme_client_attach_3.release();
		mme_client_attach_4.release();
		mme_client_detach.release();
	}	
	else if(UE_BINDING == SESSION_BASED){
		mme_client_attach_1.release();
		mme_client_detach.release();
	}
	else if(UE_BINDING == UE_BASED){
		mme_client_attach_1.release();
	}
	

}

void Ran::initial_attach() {
	pkt.clear_pkt();
	pkt.append_item(ran_ctx.imsi);
	pkt.append_item(ran_ctx.tai);
	pkt.append_item(ran_ctx.ksi_asme);
	pkt.append_item(ran_ctx.nw_capability);
	pkt.prepend_s1ap_hdr(1, pkt.len, ran_ctx.enodeb_s1ap_ue_id, 0);

	mme_client_attach_1.snd(pkt);/*XCHANGE 1*/

	TRACE(cout << "ran_initialattach:" << " request sent for ran: " << ran_ctx.imsi << endl;)
}

bool Ran::authenticate() {
	uint64_t autn_num;
	uint64_t xautn_num;
	uint64_t rand_num;
	uint64_t sqn;
	uint64_t res;
	uint64_t ck;
	uint64_t ik;

	mme_client_attach_1.rcv(pkt);/*XCHANGE 1*/

	if (pkt.len <= 0) {
		return false;
	}
	TRACE(cout << "ran_authenticate: " << " received request for ran: " << ran_ctx.imsi << endl;)
	pkt.extract_s1ap_hdr();
	ran_ctx.mme_s1ap_ue_id = pkt.s1ap_hdr.mme_s1ap_ue_id;
	pkt.extract_item(xautn_num);
	pkt.extract_item(rand_num);
	pkt.extract_item(ran_ctx.ksi_asme);

	TRACE(cout << "ran_authenticate: " << " autn: " << xautn_num << " rand: " << rand_num  << " ksiasme: " << ran_ctx.ksi_asme << ": " << ran_ctx.imsi << endl;)
	sqn = rand_num + 1;
	res = ran_ctx.key + sqn + rand_num;
	autn_num = res + 1;
	if (autn_num != xautn_num) {
		TRACE(cout << "ran_authenticate:" << " authentication of MME failure: " << ran_ctx.imsi << endl;)
		return false;
	}
	TRACE(cout << "ran_authenticate:" << " autn success: " << ran_ctx.imsi << endl;)
	ck = res + 2;
	ik = res + 3;
	ran_ctx.k_asme = ck + ik + sqn + ran_ctx.plmn_id;
	pkt.clear_pkt();
	pkt.append_item(res);
	pkt.prepend_s1ap_hdr(2, pkt.len, ran_ctx.enodeb_s1ap_ue_id, ran_ctx.mme_s1ap_ue_id);

	if(UE_BINDING==STATE_LESS){
			mme_client_attach_2.snd(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
		mme_client_attach_1.snd(pkt);
				TRACE(cout << "it does work:"<< endl;)

	}
	else if(UE_BINDING==UE_BASED){
		mme_client_attach_1.snd(pkt);
	}

	TRACE(cout << "ran_authenticate:" << " autn response sent to mme: " << ran_ctx.imsi << endl;)
	return true;
}

bool Ran::set_security() {
	uint8_t *hmac_res;
	uint8_t *hmac_xres;
	bool res;
	
	
	if(UE_BINDING==STATE_LESS){
			mme_client_attach_2.rcv(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
		mme_client_attach_1.rcv(pkt);
	}
	else if(UE_BINDING==UE_BASED){
		mme_client_attach_1.rcv(pkt);
	}
	if (pkt.len <= 0) {
		return false;
	}	

	hmac_res = g_utils.allocate_uint8_mem(HMAC_LEN);
	hmac_xres = g_utils.allocate_uint8_mem(HMAC_LEN);
	TRACE(cout << "ran_setsecurity: " << " received request for ran: " << pkt.len << ": " << ran_ctx.imsi << endl;)
	pkt.extract_s1ap_hdr();
	if (HMAC_ON) {
		g_integrity.rem_hmac(pkt, hmac_xres);
	}
	pkt.extract_item(ran_ctx.ksi_asme);
	pkt.extract_item(ran_ctx.nw_capability);
	pkt.extract_item(ran_ctx.nas_enc_algo);
	pkt.extract_item(ran_ctx.nas_int_algo);
	set_crypt_context();
	set_integrity_context();
	if (HMAC_ON) {
		g_integrity.get_hmac(pkt.data, pkt.len, hmac_res, ran_ctx.k_nas_int);
		res = g_integrity.cmp_hmacs(hmac_res, hmac_xres);
		if (res == false) {
			TRACE(cout << "ran_setsecurity:" << " hmac security mode command failure: " << ran_ctx.imsi << endl;)
			g_utils.handle_type1_error(-1, "hmac error: ran_setsecurity");
		}
	}
	TRACE(cout << "ran_setsecurity:" << " security mode command success: " << ran_ctx.imsi << endl;)
	res = true;
	pkt.clear_pkt();
	pkt.append_item(res);
	if (ENC_ON) {
		g_crypt.enc(pkt, ran_ctx.k_nas_enc);
	}
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, ran_ctx.k_nas_int);
	}
	pkt.prepend_s1ap_hdr(3, pkt.len, ran_ctx.enodeb_s1ap_ue_id, ran_ctx.mme_s1ap_ue_id);

	if(UE_BINDING==STATE_LESS){
		mme_client_attach_3.snd(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
		mme_client_attach_1.snd(pkt);

	}else if(UE_BINDING==UE_BASED){
		mme_client_attach_1.snd(pkt);
	}

	

	TRACE(cout << "ran_setsecurity:" << " security mode complete sent to mme: " << pkt.len << ": " << ran_ctx.imsi << endl;)
	free(hmac_res);
	free(hmac_xres);
	return true;
}

void Ran::set_crypt_context() {
	ran_ctx.k_nas_enc = ran_ctx.k_asme + ran_ctx.nas_enc_algo + ran_ctx.count + ran_ctx.bearer + ran_ctx.dir + 1;
}

void Ran::set_integrity_context() {
	ran_ctx.k_nas_int = ran_ctx.k_asme + ran_ctx.nas_int_algo + ran_ctx.count + ran_ctx.bearer + ran_ctx.dir + 1;
}

bool Ran::set_eps_session(TrafficMonitor &traf_mon) {
	bool res;
	uint64_t k_enodeb;
	int tai_list_size;
	

	if(UE_BINDING==STATE_LESS){
			mme_client_attach_3.rcv(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
			mme_client_attach_1.rcv(pkt);

	}else if(UE_BINDING==UE_BASED){
			mme_client_attach_1.rcv(pkt);
	}
	if (pkt.len <= 0) {
		return false;
	}
	TRACE(cout << "ran_setepssession:" << " attach accept received from mme: " << pkt.len << ": " << ran_ctx.imsi << endl;)
	pkt.extract_s1ap_hdr();
	if (HMAC_ON) {
		res = g_integrity.hmac_check(pkt, ran_ctx.k_nas_int);
		if (res == false) {
			TRACE(cout << "ran_setepssession:" << " hmac attach accept failure: " << ran_ctx.imsi << endl;)
			g_utils.handle_type1_error(-1, "hmac error: ran_setepssession");
		}
	}
	if (ENC_ON) {
		g_crypt.dec(pkt, ran_ctx.k_nas_enc);	
	}
	pkt.extract_item(ran_ctx.guti);
	pkt.extract_item(ran_ctx.eps_bearer_id);
	pkt.extract_item(ran_ctx.e_rab_id);
	pkt.extract_item(ran_ctx.s1_uteid_ul);
	pkt.extract_item(k_enodeb);
	pkt.extract_item(ran_ctx.nw_capability);
	pkt.extract_item(tai_list_size);
	pkt.extract_item(ran_ctx.tai_list, tai_list_size);
	pkt.extract_item(ran_ctx.tau_timer);
	pkt.extract_item(ran_ctx.ip_addr);
	pkt.extract_item(epc_addrs.sgw_s1_ip_addr);
	pkt.extract_item(epc_addrs.sgw_s1_port);
	pkt.extract_item(res);	
	if (res == false) {
		TRACE(cout << "ran_setepssession:" << " attach request failure: " << ran_ctx.imsi << endl;)
		return false;
	}	
	traf_mon.update_uplink_info(ran_ctx.ip_addr, ran_ctx.s1_uteid_ul, epc_addrs.sgw_s1_ip_addr, epc_addrs.sgw_s1_port);
	ran_ctx.s1_uteid_dl = ran_ctx.s1_uteid_ul;
	pkt.clear_pkt();
	pkt.append_item(ran_ctx.eps_bearer_id);
	pkt.append_item(ran_ctx.s1_uteid_dl);
	if (ENC_ON) {
		g_crypt.enc(pkt, ran_ctx.k_nas_enc);
	}
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, ran_ctx.k_nas_int);
	}
	pkt.prepend_s1ap_hdr(4, pkt.len, ran_ctx.enodeb_s1ap_ue_id, ran_ctx.mme_s1ap_ue_id);\

	/*XCHANGE 5*/
	
	if(UE_BINDING == STATE_LESS){
			mme_client_attach_4.snd(pkt);
	}	
	else if(UE_BINDING == SESSION_BASED){
			mme_client_attach_1.snd(pkt);

	}else if(UE_BINDING == UE_BASED){
			mme_client_attach_1.snd(pkt);
	}
	TRACE(cout << "ran_setepssession:" << " attach complete sent to mme: " << pkt.len << ": " << ran_ctx.imsi << endl;)
	ran_ctx.emm_state = 1;
	ran_ctx.ecm_state = 1;


	if(UE_BINDING==STATE_LESS){
			mme_client_attach_4.rcv(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
			mme_client_attach_1.rcv(pkt);
	}
	else if(UE_BINDING==UE_BASED){
			mme_client_attach_1.rcv(pkt);
	}

	return true;
}

void Ran::transfer_data(int arg_rate) {
	string cmd;
	string rate;	
	string mtu;
	string dur;
	string redir_err;
	string server_ip_addr;
	int server_port;
	rate = " -b " + to_string(arg_rate) + "M";
	mtu = " -M " + to_string(DATA_SIZE);
	dur = " -t 30"; //change this to set duration of a data plane experiment e.g -t 300 for a 5 minute data plane burst
	redir_err = " 2>&1";
	server_ip_addr = "172.16.0.2";
	server_port = ran_ctx.key + 55000;
	g_nw.add_itf(ran_ctx.key, ran_ctx.ip_addr + "/8");
	cmd = "iperf3 -B " + ran_ctx.ip_addr + " -c " + server_ip_addr + " -p " + to_string(server_port) + rate + mtu + dur + redir_err;

	cout<<cmd<<endl;

	TRACE(cout << cmd << endl;)
	system(cmd.c_str());
	TRACE(cout << "ran_transferdata:" << " transfer done for ran: " << ran_ctx.imsi << endl;)
}

bool Ran::detach() {
	uint64_t detach_type;
	bool res;

	detach_type = 1;
	pkt.clear_pkt();
	pkt.append_item(ran_ctx.guti);
	pkt.append_item(ran_ctx.ksi_asme);
	pkt.append_item(detach_type);
	if (ENC_ON)	 {
		g_crypt.enc(pkt, ran_ctx.k_nas_enc);
	}
	if (HMAC_ON) {
		g_integrity.add_hmac(pkt, ran_ctx.k_nas_int);
	}
	pkt.prepend_s1ap_hdr(5, pkt.len, ran_ctx.enodeb_s1ap_ue_id, ran_ctx.mme_s1ap_ue_id);

	if(UE_BINDING==STATE_LESS){		
		mme_client_detach.snd(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
		mme_client_detach.snd(pkt);
	}
	else if(UE_BINDING==UE_BASED){
		mme_client_attach_1.snd(pkt);
	}

	TRACE(cout << "ran_detach:" << " detach request sent to mme: " << pkt.len << ": " << ran_ctx.imsi << endl;)

	if(UE_BINDING==STATE_LESS){		
		mme_client_detach.rcv(pkt);
	}	
	else if(UE_BINDING==SESSION_BASED){
		mme_client_detach.rcv(pkt);
	}
	else if(UE_BINDING==UE_BASED){
		mme_client_attach_1.rcv(pkt);
	}

	if (pkt.len <= 0) {
		return false;
	}	
	TRACE(cout << "ran_detach:" << " detach complete received from mme: " << pkt.len << ": " << ran_ctx.imsi << endl;)
	pkt.extract_s1ap_hdr();
	if (HMAC_ON) {
		res = g_integrity.hmac_check(pkt, ran_ctx.k_nas_int);
		if (res == false) {
			TRACE(cout << "ran_detach:" << " hmac detach failure: " << ran_ctx.imsi << endl;)
			g_utils.handle_type1_error(-1, "hmac error: ran_detach");
		}
	}
	if (ENC_ON) {
		g_crypt.dec(pkt, ran_ctx.k_nas_enc);
	}
	pkt.extract_item(res);
	if (res == false) {
		TRACE(cout << "ran_detach:" << " detach failure: " << ran_ctx.imsi << endl;)
		return false;
	}
	TRACE(cout << "ran_detach:" << " detach successful: " << ran_ctx.imsi << endl;)
	return true;
}