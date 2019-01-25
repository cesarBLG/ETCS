struct Packet
{
    int NID_PACKET;
    Packet(){}
    Packet(int nid_packet)
    {
        NID_PACKET = nid_packet;
    }
};
struct EurobaliseTelegram
	{
		int Q_UPDOWN;
		int M_VERSION;
		int Q_MEDIA;
		int N_PIG;
		int N_TOTAL;
		int M_DUP;
		int M_MCOUNT;
		int NID_C;
		int NID_BG;
		int Q_LINK;
		Packet* packet;
		EurobaliseTelegram(int NID_BG, int Q_LINK, Packet* packets)
        {
            this->NID_BG = NID_BG;
            this->Q_LINK = Q_LINK;
			/*packet = new Packet[packets.Length+1];
			for(int i=0; i<packets.Length; i++)
			{
				packet[i]=packets[i];
			}
			packet[packet.Length-1]=new Packet(255);*/
		}
    };
    struct EuroradioMessage
    {
        int NID_MESSAGE;
        int L_MESSAGE;
        int T_TRAIN;
        EuroradioMessage(int NID_MESSAGE)
        {
            this->NID_MESSAGE = NID_MESSAGE;
        }
    };
    struct TrainToTrackEuroradioMessage : EuroradioMessage
    {
        int NID_ENGINE;
        TrainToTrackEuroradioMessage(int NID_MESSAGE) : EuroradioMessage(NID_MESSAGE) { }
    };
    struct TrackToTrainEuroradioMessage : EuroradioMessage
    {
        int M_ACK;
        int NID_LRBG;
        TrackToTrainEuroradioMessage(int NID_MESSAGE) : EuroradioMessage(NID_MESSAGE) { }
    };
    struct RadioMA : TrackToTrainEuroradioMessage
    {
        L2MA* MA;
        Packet* OptionalPackets;
        RadioMA(int T_TRAIN, int M_ACK, int NID_LRBG, L2MA* MA, Packet* OptionalPackets) : TrackToTrainEuroradioMessage(3)
        {
            this->T_TRAIN = T_TRAIN;
            this->M_ACK = M_ACK;
            this->NID_LRBG = NID_LRBG;
            this->MA = MA;
            this->OptionalPackets = OptionalPackets;
        }
    };
    struct UnconditionalEmergencyStop : TrackToTrainEuroradioMessage
    {
        UnconditionalEmergencyStop() : TrackToTrainEuroradioMessage(16){}
    };
    struct MARequest : TrainToTrackEuroradioMessage
    {
        int Q_MARQSTREASON;
        Packet *pck = new Packet(1);
        MARequest() : TrainToTrackEuroradioMessage(137)
        {

        }
    };
    struct MovementAuthority : Packet
    {
        int Q_DIR;
        int L_PACKET;
        int Q_SCALE;
        int V_MAIN;
        int V_LOA;
        int T_LOA;
        int N_ITER;
        int* L_SECTION;
        int* Q_SECTIONTIMER;
        int* T_SECTIONTIMER;
        int* D_SECTIONTIMERSTOPLOC;
        int L_ENDSECTION;
        int Q_ENDTIMER;
        int T_ENDTIMER;
        int D_ENDTIMERSTARTLOC;
        int Q_DANGERPOINT;
        int D_DP;
        int V_RELEASEDP;
        int Q_OVERLAP;
        int D_STARTOL;
        int T_OL;
        int D_OL;
        int V_RELEASEOL;
        MovementAuthority(int nid) : Packet(nid) { }
    };
    struct L1MA : MovementAuthority
	{
		L1MA(int q_dir, int l_packet, int q_scale, int v_main, int v_loa, int t_loa, int n_iter, int* l_section, int* q_sectiontimer, int* t_sectiontimer, int* d_sectiontimerstoploc, int l_endsection, int q_endtimer, int t_endtimer, int d_endtimerstartloc, int q_dangerpoint, int d_dp, int v_releasedp, int q_overlap, int d_startol, int t_ol, int d_ol, int v_releaseol) : MovementAuthority(12)
		{
			Q_DIR=q_dir;
			L_PACKET=l_packet;
			Q_SCALE=q_scale;
			V_MAIN=v_main;
			V_LOA=v_loa;
			T_LOA=t_loa;
			N_ITER=n_iter;
			L_SECTION=l_section;
			Q_SECTIONTIMER=q_sectiontimer;
			T_SECTIONTIMER=t_sectiontimer;
			D_SECTIONTIMERSTOPLOC=d_sectiontimerstoploc;
			L_ENDSECTION=l_endsection;
			Q_ENDTIMER=q_endtimer;
			T_ENDTIMER=t_endtimer;
			D_ENDTIMERSTARTLOC=d_endtimerstartloc;
			Q_DANGERPOINT=q_dangerpoint;
			D_DP=d_dp;
			V_RELEASEDP=v_releasedp;
			Q_OVERLAP=q_overlap;
			D_STARTOL=d_startol;
			T_OL=t_ol;
			D_OL=d_ol;
			V_RELEASEOL=v_releaseol;
		}
	};
	struct L2MA : MovementAuthority
	{
        L2MA(int q_dir, int l_packet, int q_scale, int v_main, int v_loa, int t_loa, int n_iter, int* l_section, int* q_sectiontimer, int* t_sectiontimer, int* d_sectiontimerstoploc, int l_endsection, int q_endtimer, int t_endtimer, int d_endtimerstartloc, int q_dangerpoint, int d_dp, int v_releasedp, int q_overlap, int d_startol, int t_ol, int d_ol, int v_releaseol) : MovementAuthority(15)
		{
			Q_DIR=q_dir;
			L_PACKET=l_packet;
			Q_SCALE=q_scale;
			V_MAIN=v_main;
			V_LOA=v_loa;
			T_LOA=t_loa;
			N_ITER=n_iter;
			L_SECTION=l_section;
			Q_SECTIONTIMER=q_sectiontimer;
			T_SECTIONTIMER=t_sectiontimer;
			D_SECTIONTIMERSTOPLOC=d_sectiontimerstoploc;
			L_ENDSECTION=l_endsection;
			Q_ENDTIMER=q_endtimer;
			T_ENDTIMER=t_endtimer;
			D_ENDTIMERSTARTLOC=d_endtimerstartloc;
			Q_DANGERPOINT=q_dangerpoint;
			D_DP=d_dp;
			V_RELEASEDP=v_releasedp;
			Q_OVERLAP=q_overlap;
			D_STARTOL=d_startol;
			T_OL=t_ol;
			D_OL=d_ol;
			V_RELEASEOL=v_releaseol;
			if(V_MAIN==0) N_ITER=0;
		}
	};
	struct InternationalStaticSpeedProfile : Packet
	{
        int Q_DIR;
        int L_PACKET;
        int Q_SCALE;
        int* D_STATIC;
        int* V_STATIC;
        int* Q_FRONT;
        int* N_ITER;
        int* Q_DIFF;
        int* NC_CDDIFF;
        int* NC_DIFF;
        int* V_DIFF;
		InternationalStaticSpeedProfile( int q_dir, int l_packet, int q_scale, int* d_static, int* v_static, int* q_front, int* n_iter, int* q_diff, int* nc_cddif, int* nc_diff, int* v_diff) : Packet(27)
		{
            NID_PACKET = 27;
			Q_DIR=q_dir;
			L_PACKET=l_packet;
			Q_SCALE=q_scale;
			D_STATIC=d_static;
			V_STATIC=v_static;
			Q_FRONT=q_front;
			N_ITER=n_iter;
			Q_DIFF=q_diff;
			NC_CDDIFF=nc_cddif;
			NC_DIFF=nc_diff;
			V_DIFF=v_diff;
		}
	};
	struct TemporarySpeedRestriction : Packet
	{
        int Q_DIR;
        int L_PACKET;
        int Q_SCALE;
        int NID_TSR;
        int D_TSR;
        int L_TSR;
        int Q_FRONT;
        int V_TSR;
        TemporarySpeedRestriction(int q_dir, int l_packet, int q_scale, int nid_tsr, int d_tsr, int l_tsr, int q_front, int v_tsr) : Packet(65)
        {
            Q_DIR = q_dir;
            L_PACKET = l_packet;
            Q_SCALE = q_scale;
            NID_TSR = nid_tsr;
            D_TSR = d_tsr;
            L_TSR = l_tsr;
            Q_FRONT = q_front;
            V_TSR = v_tsr;
        }
	};
	struct TemporarySpeedRestrictionRevocation : Packet
	{
        int Q_DIR;
        int L_PACKET;
        int NID_TSR;
		TemporarySpeedRestrictionRevocation(int q_dir, int l_packet, int nid_tsr) : Packet(66)
        {
            Q_DIR = q_dir;
            L_PACKET = l_packet;
            NID_TSR = nid_tsr;
        }
	};
	struct ModeProfile : Packet
	{
        int Q_DIR;
        int L_PACKET;
        int Q_SCALE;
        int N_ITER;
        int* D_MAMODE;
        int* M_MAMODE;
        int* V_MAMODE;
        int* L_MAMODE;
        int* L_ACKMAMODE;
        int* Q_MAMODE;
		ModeProfile(int q_dir, int l_packet, int q_scale, int n_iter, int* d_mamode, int* m_mamode, int* v_mamode, int* l_mamode, int* l_ackmamode, int* q_mamode) : Packet(80)
		{
            Q_DIR = q_dir;
            L_PACKET = l_packet;
            Q_SCALE = q_scale;
			N_ITER=n_iter;
			D_MAMODE=d_mamode;
			M_MAMODE=m_mamode;
			V_MAMODE=v_mamode;
			L_MAMODE=l_mamode;
			L_ACKMAMODE=l_ackmamode;
			Q_MAMODE=q_mamode;
		}
	};
    struct MARequestParameters : Packet
    {
        int Q_DIR;
        int L_PACKET;
        int T_MAR;
        int T_TIMEOUTREQST;
        int T_CYCRQST;
        MARequestParameters(int Q_DIR, int L_PACKET, int T_MAR, int T_TIMEOUTREQST,int T_CYCRQST) : Packet(57)
        {
            this->Q_DIR = Q_DIR;
            this->L_PACKET = L_PACKET;
            this->T_MAR = T_MAR;
            this->T_TIMEOUTREQST = T_TIMEOUTREQST;
            this->T_CYCRQST = T_CYCRQST;
        }
    };
    struct InfillLocationReference : Packet
    {
        int Q_DIR;
        int L_PACKET;
        int Q_NEWCOUNTRY;
        int NID_C;
        int NID_BG;
        InfillLocationReference(int Q_DIR, int L_PACKET, int Q_NEWCOUNTRY, int NID_C, int NID_BG) : Packet(136)
        {
            this->Q_DIR = Q_DIR;
            this->L_PACKET = L_PACKET;
            this->Q_NEWCOUNTRY = Q_NEWCOUNTRY;
            this->NID_C = NID_C;
            this->NID_BG = NID_BG;
        }
    };
    struct Linking : Packet
    {
        int Q_DIR;
        int L_PACKET;
        int Q_SCALE;
        int D_LINK;
        int Q_NEWCOUNTRY;
        int NID_C;
        int NID_BG;
        int Q_LINKORIENTATION;
        int Q_LINKREACTION;
        int Q_LOCACC;
        Linking(int Q_DIR, int L_PACKET, int Q_SCALE, int D_LINK, int Q_NEWCOUNTRY, int NID_C, int NID_BG, int Q_LINKORIENTATION, int Q_LINKREACTION, int Q_LOCACC) : Packet(5)
        {
            this->Q_DIR = Q_DIR;
            this->L_PACKET = L_PACKET;
            this->Q_SCALE = Q_SCALE;
            this->D_LINK = D_LINK;
            this->Q_NEWCOUNTRY = Q_NEWCOUNTRY;
            this->NID_C = NID_C;
            this->NID_BG = NID_BG;
            this->Q_LINKORIENTATION = Q_LINKORIENTATION;
            this->Q_LINKREACTION = Q_LINKREACTION;
            this->Q_LOCACC = Q_LOCACC;
        }
    };
