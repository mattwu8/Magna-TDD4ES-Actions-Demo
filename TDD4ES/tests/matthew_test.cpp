#define UNIT_TEST
#include "CppUTest/TestHarness.h"
#include "Mock.h"



vx_enum VX_TARGET_STRING;
const char* TIVX_TARGET_MCU2_0;



//Implementation of write_output_image_yuv422_8bit function
vx_int32 write_output_image_yuv422_8bit(char * file_name, vx_image out_yuv)
{
    FILE * fp = fopen(file_name, "wb");
    if(!fp)
    {
        APP_PRINTF("Unable to open file %s\n", file_name);
        return -1;
    }
    vx_uint32 len1 = write_output_image_fp(fp, out_yuv);
    fclose(fp);
    APP_PRINTF("%d bytes written to %s\n", len1, file_name);
    return len1;
}

TEST_GROUP(write_output_image_yuv422_8bit)
{
    void setup() {
    }

    void teardown() {
                mock().clear();
    }
};

TEST(write_output_image_yuv422_8bit, PassWriteFile)
{   
    const char* file_name = "test_img.jpeg";
    vx_image test_image = nullptr;

    mock().expectOneCall("write_output_image_fp").withParameter("out_nv12", test_image).ignoreOtherParameters().andReturnValue(1024);
    int result = write_output_image_yuv422_8bit((char*)file_name, test_image);
    CHECK_EQUAL(1024, result);

    mock().checkExpectations();

    //test
}

TEST(write_output_image_yuv422_8bit, FailOpenFile) {
    vx_image mock_image = nullptr;
    const char* invalid_file = "";  
    
    int result = write_output_image_yuv422_8bit((char*)invalid_file, mock_image);

    CHECK_EQUAL(-1, result);


    mock().checkExpectations();
}
//implementation of app_send_test_frame function

vx_status app_send_test_frame(vx_node cap_node, tivx_raw_image raw_img)
{
    vx_status status = VX_SUCCESS;

    status = tivxCaptureRegisterErrorFrame(cap_node, (vx_reference)raw_img);

    return status;
}

TEST_GROUP(app_send_test_frame)
{
    void setup() {
    }

    void teardown() {
        mock().clear();
    }
};

TEST(app_send_test_frame, Success)
{   vx_node cap_node = nullptr;
    tivx_raw_image raw_img = nullptr;

    mock().expectOneCall("tivxCaptureRegisterErrorFrame").withParameter("cap_node", cap_node).withParameter("raw_img",raw_img).andReturnValue(VX_SUCCESS);

    vx_status status = app_send_test_frame(cap_node, raw_img);

    CHECK_EQUAL(VX_SUCCESS, status);

    mock().checkExpectations();
}

TEST(app_send_test_frame, Fail)
{   vx_node cap_node = nullptr;
    tivx_raw_image raw_img = nullptr;

    mock().expectOneCall("tivxCaptureRegisterErrorFrame").withParameter("cap_node", cap_node).withParameter("raw_img",raw_img).andReturnValue(VX_FAILURE);

    vx_status status = app_send_test_frame(cap_node, raw_img);

    CHECK_EQUAL(VX_FAILURE, status);

    mock().checkExpectations();
}

//implementation of app_create_aewb function
vx_status app_create_aewb(AppObj *obj, uint32_t sensor_wdr_mode)
{
    vx_status status = VX_SUCCESS;
    int32_t dcc_buff_size = 0;
    int32_t dcc_buff_size_driver = 0;

#ifdef __x86_64__
    int32_t dcc_buff_size_fs = 0;
#endif

    const vx_char dcc_2a_user_data_object_name[] = "dcc_2a";
    uint8_t * dcc_2a_buf;
    vx_map_id dcc_2a_buf_map_id;
    vx_int32 sensor_dcc_enabled = 1;

    obj->aewb_cfg.sensor_dcc_id = obj->cam_dcc_id;
    obj->aewb_cfg.sensor_img_format = 0;
    obj->aewb_cfg.sensor_img_phase = 3;
    obj->aewb_cfg.awb_num_skip_frames = 0;
    obj->aewb_cfg.ae_num_skip_frames = 0;
    obj->aewb_cfg.channel_id = obj->selectedCam;

    obj->aewb_config = vxCreateUserDataObject(obj->context, "tivx_aewb_config_t", sizeof(tivx_aewb_config_t), &obj->aewb_cfg);

    if(sensor_dcc_enabled)
    {
        dcc_buff_size_driver = appIssGetDCCSize2A(obj->sensor_name, sensor_wdr_mode);

        if(dcc_buff_size_driver > 0)
        {
            dcc_buff_size += dcc_buff_size_driver;
        }
        else
        {
            dcc_buff_size_driver = 0;
        }

#ifdef __x86_64__
        dcc_buff_size_fs = obj->fs_dcc_numbytes_2a;

        if(dcc_buff_size_fs > 0)
        {
            dcc_buff_size += dcc_buff_size_fs;
        }

#endif

        if(dcc_buff_size<=0)
        {
            printf("Invalid DCC size for 2A. Disabling DCC \n");
            obj->dcc_param_2a = NULL;
        }
        else
        {
            obj->dcc_param_2a = vxCreateUserDataObject(
                obj->context,
                (const vx_char*)&dcc_2a_user_data_object_name,
                dcc_buff_size,
                NULL
            );
            if(status == VX_SUCCESS)
            {
                status = vxMapUserDataObject(
                                    obj->dcc_param_2a,
                                    0,
                                    dcc_buff_size,
                                    &dcc_2a_buf_map_id,
                                    (void **)&dcc_2a_buf,
                                    VX_WRITE_ONLY,
                                    VX_MEMORY_TYPE_HOST,
                                    0
                                );
            }

            if(status == VX_SUCCESS)
            {
                status = appIssGetDCCBuff2A(obj->sensor_name, sensor_wdr_mode,  dcc_2a_buf, dcc_buff_size_driver);
                if(status != VX_SUCCESS)
                {
                    printf("Couldn't get 2A DCC buffer from sensor driver \n");
                    status = VX_SUCCESS;
                }

#ifdef __x86_64__
                if(dcc_buff_size_fs> 0)
                {
                    memcpy(dcc_2a_buf+dcc_buff_size_driver, obj->fs_dcc_buf_2a, dcc_buff_size_fs);
                }
#endif
            }
            vxUnmapUserDataObject(obj->dcc_param_2a, dcc_2a_buf_map_id);
        }
    }
    else
    {
        obj->dcc_param_2a = NULL;
    }
    obj->ae_awb_result =
            vxCreateUserDataObject(obj->context, "tivx_ae_awb_params_t",
            sizeof(tivx_ae_awb_params_t), NULL);
    if (vxGetStatus((vx_reference)obj->ae_awb_result) != VX_SUCCESS)
    {
        APP_PRINTF("obj->ae_awb_result) create failed\n");
        return VX_FAILURE;
    }

    obj->node_aewb = tivxAewbNode(obj->graph,
                                      obj->aewb_config,
                                      obj->histogram,
                                      obj->h3a_aew_af,
                                      NULL,
                                      obj->ae_awb_result,
                                      obj->dcc_param_2a);
    vxSetNodeTarget(obj->node_aewb, VX_TARGET_STRING, TIVX_TARGET_MCU2_0);
    tivxSetNodeParameterNumBufByIndex(obj->node_aewb, 4u, NUM_BUFS);
    if(NULL != obj->node_aewb)
    {
        vxSetReferenceName((vx_reference)obj->node_aewb, "2A_AlgNode");
    }
    else
    {
        APP_PRINTF("tivxAewbNode returned NULL \n");
        status = VX_FAILURE;
    }
    APP_PRINTF("AEWB Set Reference done\n");
    return status;
}

TEST_GROUP(app_create_aewb_test)
{
    void setup() {
    }

    void teardown() {
        mock().clear();
    }
};

TEST(app_create_aewb_test, FailureAtvxMapUserDataObject)
{   



    AppObj obj;
    uint32_t sensor_wdr_mode;

    vx_user_data_object ae_awb_result_ptr = nullptr;
    vx_user_data_object aewb_config_ptr = nullptr;
    vx_user_data_object dcc_param_2a_ptr= nullptr;
    vx_node node;
    int test_int =10;
    node = &test_int;

    //check that all expected functions all called, and all expected parameters values are correct
    //testing that everything is expected when vxMapUserDataObject returns VX_FAILURE

    //vxCreateUserDataObject returns aewb_config_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(aewb_config_ptr);

    //appIssGetDCCSize2A returns 7, meaning dcc_buff_size_driver =7
    mock().expectOneCall("appIssGetDCCSize2A").ignoreOtherParameters().andReturnValue(7);

    //expects that size = 7, returns dcc_param_2a_ptr
    mock().expectOneCall("vxCreateUserDataObject").withParameter("size",7).ignoreOtherParameters().andReturnValue(dcc_param_2a_ptr);
    
    //expects that size = 7, obj = dcc_param_2a_ptr, returns VX_FAILURE
    mock().expectOneCall("vxMapUserDataObject").withParameter("size",7).withParameter("obj",dcc_param_2a_ptr).ignoreOtherParameters().andReturnValue(VX_FAILURE);
    
    //expects that obj = dcc_param_2a_ptr
    mock().expectOneCall("vxUnmapUserDataObject").withParameter("obj",dcc_param_2a_ptr).ignoreOtherParameters();
    
    //returns ae_awb_result_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(ae_awb_result_ptr);
    
    //expects that reference = (vx_reference)ae_awb_result_ptr, returns VX_SUCCESS
    mock().expectOneCall("vxGetStatus").withParameter("reference",(vx_reference)ae_awb_result_ptr).andReturnValue(VX_SUCCESS);
    
    //expects that configuration = aewb_config_ptr, ae_awb_result = ae_awb_result_ptr, dcc_param = dcc_param_2a_ptr, returns node
    mock().expectOneCall("tivxAewbNode").withParameter("configuration",aewb_config_ptr).withParameter("ae_awb_result",ae_awb_result_ptr).withParameter("dcc_param",dcc_param_2a_ptr).ignoreOtherParameters().andReturnValue(node);
    
    //expects that node = node
    mock().expectOneCall("vxSetNodeTarget").withParameter("node",node).ignoreOtherParameters();

    //expects that node = node
    mock().expectOneCall("tivxSetNodeParameterNumBufByIndex").withParameter("node",node).ignoreOtherParameters();

    //expects that ref = (vx_reference)node
    mock().expectOneCall("vxSetReferenceName").withParameter("ref", (vx_reference)node).ignoreOtherParameters();
    
    //calls function, stores status into status variable
    vx_status status = app_create_aewb(&obj, sensor_wdr_mode);

    //assertions
    CHECK_EQUAL(-1, status);
    CHECK_EQUAL(dcc_param_2a_ptr, obj.dcc_param_2a);
    CHECK_EQUAL(node, obj.node_aewb);
    CHECK_EQUAL(ae_awb_result_ptr, obj.ae_awb_result);

    //verify mocks
    mock().checkExpectations();
}

TEST(app_create_aewb_test, dcc_buff_size_Equal0)
{   

    AppObj obj;
    uint32_t sensor_wdr_mode;

    vx_user_data_object ae_awb_result_ptr = nullptr;
    vx_user_data_object aewb_config_ptr = nullptr;
    vx_user_data_object dcc_param_2a_ptr= nullptr;
    vx_node node;
    int test_int =10;
    node = &test_int;

    //check that all expected functions all called, and all expected parameters values are correct
    //testing that everything is expected when dcc_buff_size = 0

    //vxCreateUserDataObject returns aewb_config_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(aewb_config_ptr);

    //appIssGetDCCSize2A returns 0, meaning dcc_buff_size_driver =0
    mock().expectOneCall("appIssGetDCCSize2A").ignoreOtherParameters().andReturnValue(0);

    //returns ae_awb_result_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(ae_awb_result_ptr);

    //expects that reference = (vx_reference)ae_awb_result_ptr, returns VX_SUCCESS
    mock().expectOneCall("vxGetStatus").withParameter("reference",(vx_reference)ae_awb_result_ptr).andReturnValue(VX_SUCCESS);

    //expects that configuration = aewb_config_ptr, ae_awb_result = ae_awb_result_ptr, returns node
    mock().expectOneCall("tivxAewbNode").withParameter("configuration",aewb_config_ptr).withParameter("ae_awb_result",ae_awb_result_ptr).ignoreOtherParameters().andReturnValue(node);
    
    //expects that node = node
    mock().expectOneCall("vxSetNodeTarget").withParameter("node",node).ignoreOtherParameters();

    //expects that node node
    mock().expectOneCall("tivxSetNodeParameterNumBufByIndex").withParameter("node",node).ignoreOtherParameters();

    //expects that ref = (vx_reference)node
    mock().expectOneCall("vxSetReferenceName").withParameter("ref", (vx_reference)node).ignoreOtherParameters();
    
    //calls function, stores status into status variable
    vx_status status = app_create_aewb(&obj, sensor_wdr_mode);

    //assertions
    CHECK_EQUAL(0, status);
    CHECK_EQUAL(nullptr, obj.dcc_param_2a); //obj.dcc_param_2a should be set to NULL
    CHECK_EQUAL(node, obj.node_aewb);
    CHECK_EQUAL(ae_awb_result_ptr, obj.ae_awb_result);

    //verify mocks
    mock().checkExpectations();
}

TEST(app_create_aewb_test, appIssGetDCCBuff2ANotSucess)
{   
    AppObj obj;
    uint32_t sensor_wdr_mode;

    vx_user_data_object ae_awb_result_ptr = nullptr;
    vx_user_data_object aewb_config_ptr = nullptr;
    vx_user_data_object dcc_param_2a_ptr= nullptr;
    vx_node node;
    int test_int =10;
    node = &test_int;

    //check that all expected functions all called, and all expected parameters values are correct
    //testing that everything is expected when appIssGetDCCBuff2A returns VX_FAILURE
//test
    //vxCreateUserDataObject returns aewb_config_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(aewb_config_ptr);

    //appIssGetDCCSize2A returns 7, meaning dcc_buff_size_driver = 7
    mock().expectOneCall("appIssGetDCCSize2A").ignoreOtherParameters().andReturnValue(7);

    //expects that size = 7, returns dcc_param_2a_ptr    
    mock().expectOneCall("vxCreateUserDataObject").withParameter("size",7).ignoreOtherParameters().andReturnValue(dcc_param_2a_ptr);
    
    //expects that size = 7, obj = dcc_param_2a_ptr, returns VX_SUCCESS
    mock().expectOneCall("vxMapUserDataObject").withParameter("size",7).withParameter("obj",dcc_param_2a_ptr).ignoreOtherParameters().andReturnValue(VX_SUCCESS);
    
    //expects that num_bytes = 7, returns VX_FAILURE
    mock().expectOneCall("appIssGetDCCBuff2A").withParameter("num_bytes",7).ignoreOtherParameters().andReturnValue(VX_FAILURE);
    
    //expects that obj = dcc_param_2a_ptr
    mock().expectOneCall("vxUnmapUserDataObject").withParameter("obj",dcc_param_2a_ptr).ignoreOtherParameters(); 
    
    //returns ae_awb_result_ptr
    mock().expectOneCall("vxCreateUserDataObject").ignoreOtherParameters().andReturnValue(ae_awb_result_ptr);
    
    //expects that reference = (vx_reference)ae_awb_result_ptr, returns VX_SUCCESS
    mock().expectOneCall("vxGetStatus").withParameter("reference",(vx_reference)ae_awb_result_ptr).andReturnValue(VX_SUCCESS);
    
    //expects that configuration = aewb_config_ptr, ae_awb_result = ae_awb_result_ptr, dcc_param = dcc_param_2a_ptr, returns node
    mock().expectOneCall("tivxAewbNode").withParameter("configuration",aewb_config_ptr).withParameter("ae_awb_result",ae_awb_result_ptr).ignoreOtherParameters().andReturnValue(node);
    
    //expects that node = node
    mock().expectOneCall("vxSetNodeTarget").withParameter("node",node).ignoreOtherParameters();
    
    //expects that node = node
    mock().expectOneCall("tivxSetNodeParameterNumBufByIndex").withParameter("node",aewb_config_ptr).ignoreOtherParameters();
    
    //expects that ref = (vx_reference)node
    mock().expectOneCall("vxSetReferenceName").withParameter("ref", (vx_reference)node).ignoreOtherParameters();
    
    //calls function, stores status into status variable
    vx_status status = app_create_aewb(&obj, sensor_wdr_mode);

    //assertions

    CHECK_EQUAL(0, status);
    CHECK_EQUAL(nullptr, obj.dcc_param_2a);
    CHECK_EQUAL(node, obj.node_aewb);
    CHECK_EQUAL(ae_awb_result_ptr, obj.ae_awb_result);
    
    //verify mocks
    mock().checkExpectations();
}