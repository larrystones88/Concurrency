/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

static uint32 client_id = 0;
static gchar revision_id[REVISION_ID_SIZE_MAX] = {0};
pthread_t client_tid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


/**
* @brief RDBI_NADIF_callback
*
* @description receive Nadif data such as revision id
*
* @param[out]rqst_data            Output data to response to VuC
*
*
**/
static uint8 RDBI_NADIF_callback(NOTIFICATION_REQUEST_DATA *rqst_data, uint32 size, void *data)
{
    NADIF_GET_REVISION_ID_RESP *rev_id;

    if (NULL == rqst_data || NULL == data) {
        tpLOG_info("%s: Invalid callback arguments", __FUNCTION__);
        return EINVAL;
    }

    tpLOG_info("%s: Response received, feat_id: %i, client id: %i.\n", __FUNCTION__, rqst_data->feat_id, rqst_data->sender_id);

    switch(rqst_data->feat_id)
    {
       case NADIF_FEAT_MODEM_GET_REVISION_ID:
            pthread_mutex_lock(&mutex);
            rev_id = (NADIF_GET_REVISION_ID_RESP*)(data);
            tpLOG_info("revision_id = %s", rev_id->revision_id);
            strncat(revision_id,(gchar*)rev_id->revision_id,REVISION_ID_SIZE_MAX-1);
            pthread_mutex_unlock(&mutex);
            pthread_cond_signal(&cond);
            break;
        default:
            break;
    }
    return 0;
}

/**
* @brief get diff parameters from revision id
*
* @description parse revision id
*
* @param[out]modem_sw            Output data to response to VuC
*
*
**/
static gboolean RDBI_NADIF_get_modem_sw_field(gchar* revision, gchar* modem_sw,gchar* other1,gchar* other2)
{
    int end_size = strlen(REPLACE_END);

    //"MDM9650.LE.2.3-00050-NBOOT.AUTONTCH.NEFS.2KNAND.PROD-4"
    gchar* pMDM_start = strchr(revision,'\"');
    if(NULL == pMDM_start) return FALSE;

    gchar* pMDM_end = strchr(pMDM_start+1,'\"');
    if(NULL == pMDM_end) return FALSE;

    int lenMDM = pMDM_end-pMDM_start+1;
    if(lenMDM > RDBI_NADIF_MDM_SIZE_MAX)
    {
        strncat(modem_sw,revision,RDBI_NADIF_MDM_SIZE_MAX);//copy the max size of str
        strncat(modem_sw+RDBI_NADIF_MDM_SIZE_MAX-end_size,REPLACE_END,end_size);//end with connection bytes
        strncat(modem_sw,",",1);//connector
    }
    else
    {
        strncat(modem_sw,revision,lenMDM+1); //add ,
    }
    //"r00050.4-14-g041a35a",
    gchar* pParam_start = strchr(pMDM_end+1,'\"');
    if(pParam_start == NULL) return FALSE;
    gchar* pParam_end = strchr(pParam_start+1,'\"');
    if(pParam_end == NULL) return FALSE;
    int lenParam = pParam_end-pParam_start+1;
    if(lenParam >RDBI_NADIF_PARAM2_SIZE_MAX)
    {
        strncat(modem_sw,pParam_start,RDBI_NADIF_PARAM2_SIZE_MAX);//full copy
        strncat(modem_sw+RDBI_NADIF_PARAM2_SIZE_MAX-end_size,REPLACE_END,end_size);
    }
    else
    {
        strncat(modem_sw,pParam_start,lenParam);//do not add ,
    }
    //"refs/heads/drt-entry-2.y",gchar
    gchar drt_ver[RDBI_NADIF_DRT_VER_SIZE_MAX] = "\"";//make it a string and save it first
    gchar* pDRT_start = strchr(pParam_end+1,'\"');
    if(pDRT_start ==  NULL) return FALSE;//incomplete str
    gchar* pDRT_end = strchr(pDRT_start+1,'\"');
    if(pDRT_end == NULL) return FALSE;
    int headSize = strlen("\"refs/heads/drt-");
    int lenDRT =pDRT_end-pDRT_start+1;
    if(lenDRT > RDBI_NADIF_DRT_VER_SIZE_MAX + headSize)
    {
        strncat(drt_ver,pDRT_start+headSize,RDBI_NADIF_DRT_VER_SIZE_MAX-1);//add "
        strncat(drt_ver+RDBI_NADIF_DRT_VER_SIZE_MAX-end_size,REPLACE_END,end_size);//replace end
    }
    else
    {
        strncat(drt_ver,pDRT_start+headSize,lenDRT-strlen("\"refs/heads/drt-")+1);
    }

    //"MODEM9x28_37.01_2018-11-14",
    gchar* pModem_start = strchr(pDRT_end+1,'\"');
    if(pModem_start == NULL) return FALSE;
    gchar* pModem_end = strchr(pModem_start+1,'\"');
    if(pModem_end == NULL) return FALSE;
    int lenModem = pModem_end-pModem_start+1;
    if(lenModem > RDBI_NADIF_MODEM_SIZE_MAX)
    {
        strncat(other1,pModem_start,RDBI_NADIF_MODEM_SIZE_MAX);//full copy
        strncat(other1+RDBI_NADIF_MODEM_SIZE_MAX-end_size,REPLACE_END,end_size);//replace
        strncat(other1,",",1);//connector
    }
    else
    {
        strncat(other1,pModem_start,lenModem+1);//add ","
    }

    //"QCN_BL28EU-001_00.13",
    gchar* pQcn_start = strchr(pModem_end+1,'\"');
    if(pQcn_start == NULL) return FALSE;
    gchar* pQcn_end = strchr(pQcn_start+1,'\"');
    if(pQcn_end == NULL) return FALSE;
    int lenQcn = pQcn_end-pQcn_start+1;
    if(lenQcn > RDBI_NADIF_QCN_SIZE_MAX)
    {
        strncat(other1,pQcn_start,RDBI_NADIF_QCN_SIZE_MAX);//full copy
        strncat(other1+RDBI_NADIF_QCN_SIZE_MAX-end_size,REPLACE_END,end_size);//replace
    }
    else
    {
        strncat(other1,pQcn_start,lenQcn);//not add ","
    }

    //"MCFG_HW_BL28_00.15",
    gchar* pMcfg_start = strchr(pQcn_end+1,'\"');
    if(pMcfg_start == NULL) return FALSE;
    gchar* pMcfg_end = strchr(pMcfg_start+1,'\"');
    if(pMcfg_end == NULL) return FALSE;
    int lenMcfg = pMcfg_end-pMcfg_start+1;
    if(lenMcfg > RDBI_NADIF_MCFG_SIZE_MAX)
    {
        strncat(other2,pMcfg_start,RDBI_NADIF_MCFG_SIZE_MAX);
        strncat(other2+RDBI_NADIF_MCFG_SIZE_MAX-end_size,REPLACE_END,end_size);//replace
        strncat(other2,",",1);//connector
    }
    else
    {
        strncat(other2,pMcfg_start,lenMcfg+1);//add  ,
    }
    ///"UNKNOWN",
    gchar* pCarrier_start = strchr(pMcfg_end+1,'\"');
    if(pCarrier_start == NULL) return FALSE;
    gchar* pCarrier_end = strchr(pCarrier_start+1,'\"');
    if(pCarrier_end == NULL) return FALSE;
    int lenCarrier = pCarrier_end-pCarrier_start+1;
    if(lenCarrier > RDBI_NADIF_CARRIER_SIZE_MAX)
    {
        strncat(other2,pCarrier_start,RDBI_NADIF_CARRIER_SIZE_MAX);
        strncat(other2+RDBI_NADIF_CARRIER_SIZE_MAX-end_size,REPLACE_END,end_size);
        strncat(other2,",",1);//connector
    }
    else
    {
        strncat(other2,pCarrier_start,lenCarrier+1);
    }
    //"0x0501081F"
    gchar* pSerial_start = strchr(pCarrier_end+1,'\"');
    if(pSerial_start == NULL) return FALSE;
    gchar* pSerial_end = strchr(pSerial_start+1,'\"');
    if(pSerial_end == NULL) return FALSE;
    int lenSerial = pSerial_end-pSerial_start+1;
    if(lenSerial > RDBI_NADIF_SERIAL_SIZE_MAX)
    {
        strncat(other2,pSerial_start,RDBI_NADIF_SERIAL_SIZE_MAX);
        strncat(other2+RDBI_NADIF_SERIAL_SIZE_MAX-end_size,REPLACE_END,end_size);
    }
    else
    {
        strncat(other2,pSerial_start,lenSerial+1);
    }
    strncat(other2,",",1);//connector

    strncat(other2,drt_ver,RDBI_NADIF_DRT_VER_SIZE_MAX);

    return TRUE;
}

/**
* @brief get revision id
*
* @description retrieve the modem version from soc
*
* @param[out]puc_out_msg                Output buffer with the response to be sent back to the VUC
*
*
**/
static gboolean RDBI_NADIF_modem_get_revision_id(guint8* puc_out_msg)
{
    gchar modem_sw[RDBI_TCU_SW_VERSION_MODEM_SIZE] = {0};
    gchar other1[RDBI_TCU_SW_VERSION_OTHER1_SIZE] = {0};
    gchar other2[RDBI_TCU_SW_VERSION_OTHER2_SIZE] = {0};
    NOTIFICATION_REQUEST_DATA rqst;
    gboolean ret = FALSE;
    NADIF_STATUS status;
    struct timespec timeToWait = {0};
    struct timeval now = {0};

    if(NADIF_init(RDBI_NADIF_callback, &client_id) != 1)
    {
        tpLOG_info("%s:Not able to init NADIF", __FUNCTION__);
    }
    else{
        tpLOG_info("%s: clientitd = %d", __FUNCTION__,(int)client_id);
        pthread_create(&client_tid,
                           NULL,
                           (void*)NADIF_main,
                           NULL);
    }

    rqst.sender_id = client_id;
    status = NADIF_modem_get_revision_id(&rqst, revision_id);
    if(status == NADIF_STATUS_SUCCESS)
    {
        tpLOG_info("NADIF_modem_get_revision_id success, rev id: %s.", revision_id);
        gettimeofday(&now,NULL);
        timeToWait.tv_sec = now.tv_sec+10;
        //timeToWait.tv_nsec = (now.tv_usec+1000UL*0)*1000UL; if need the msec
        pthread_mutex_lock(&mutex);//add lock
        pthread_cond_timedwait(&cond,&mutex,&timeToWait);// wait until NADIF callback finish or timeout
        if(TRUE == RDBI_NADIF_get_modem_sw_field(revision_id,modem_sw,other1,other2))
        {
                strncpy((gchar*)&puc_out_msg[RDBI_TCU_SW_VERSION_MODEM_INDEX],modem_sw,RDBI_TCU_SW_VERSION_MODEM_SIZE);
                strncpy((gchar*)&puc_out_msg[RDBI_TCU_SW_VERSION_OTHER1_INDEX],other1,RDBI_TCU_SW_VERSION_OTHER1_SIZE);
                strncpy((gchar*)&puc_out_msg[RDBI_TCU_SW_VERSION_OTHER2_INDEX],other2,RDBI_TCU_SW_VERSION_OTHER2_SIZE);
                ret = TRUE;
        }
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        tpLOG_info("NADIF_modem_get_revision_id failure, client id: %d.", (int)client_id);
    }
    NADIF_release(client_id);

    return ret;

}