#ifndef MOCK_H
#define MOCK_H

#include <cstdio>
#include <cstring>  // For strcmp
#include <cstdlib>  // For atoi
#include <CppUTestExt/MockSupport.h>  // For mocking support

// Mock APP_PRINTF for testing
#define APP_PRINTF printf

// Type definitions required for testing
typedef int vx_status;
typedef unsigned int vx_uint32;
typedef char vx_char;
typedef void* vx_context;
typedef void* vx_image;
typedef void* vx_user_data_object;
typedef void* vx_graph;
typedef void* vx_node;
typedef void* vx_reference;
typedef void* vx_df_image;
typedef void* tivx_raw_image;
typedef void* tivx_ae_awb_params_t;
typedef void* vx_distribution;
typedef int vx_map_id;
typedef size_t vx_size;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

// Define missing types
typedef int vx_int32;
typedef unsigned char vx_bool;  // Use unsigned char for boolean type
typedef char vx_int8;
typedef void* vx_enum;
// Define boolean values
#define vx_true_e  ((vx_bool)1)
#define vx_false_e ((vx_bool)0)

// Constants
#define VX_SUCCESS 0
#define VX_FAILURE -1
#define VX_WRITE_ONLY 0
#define VX_MEMORY_TYPE_HOST 0
#define VX_DF_IMAGE_NV12 0x15
#define NUM_FRAMES 100
#define tivx_vpac_viss_params_t 99
#define tivx_h3a_data_t 98
//inline vx_status vxSetNodeTarget(vx_node node, vx_enum target_enum, const char *target_string)


#define NUM_BUFS 1
typedef struct
{
    uint16_t sensor_dcc_id; /*!<DCC ID of the sensor. Must correspond to the value specified in tuning XML files */
    uint8_t sensor_img_format; /*!<Image Format : BAYER = 0x0, Rest unsupported */
    uint8_t sensor_img_phase; /*!<Image Format : BGGR = 0, GBRG = 1, GRBG = 2, RGGB = 3 */
    uint8_t awb_mode; /*!<AWB Mode : 0 = Auto, 1 = Manual, 2 = Disabled */
    uint8_t ae_mode; /*!<AE Mode : 0 = Auto, 1 = Manual, 2 = Disabled */
    uint8_t awb_num_skip_frames; /*!<0 = Process every frame */
    uint8_t ae_num_skip_frames; /*!<0 = Process every frame */
    uint8_t channel_id; /*!<channel ID */
}tivx_aewb_config_t;
// Define AppObj structure as per the original code
typedef struct {
    vx_context context;
    unsigned int width_in;
    unsigned int height_in;
    int cam_dcc_id;
    vx_image y8_r8_c2;
    vx_image uv8_g8_c3;
    vx_image y12;
    vx_image uv12_c1;
    vx_image s8_b8_c4;
    vx_user_data_object histogram;
    vx_user_data_object configuration;
    vx_user_data_object h3a_aew_af;
    vx_user_data_object dcc_param_viss;
    vx_graph graph;
    vx_node node_viss;
    vx_image raw;
    tivx_aewb_config_t aewb_cfg;
    vx_user_data_object aewb_config;
    vx_user_data_object dcc_param_2a;
    vx_user_data_object ae_awb_result;
    tivx_ae_awb_params_t ae_awb_params;
    vx_node  node_aewb;
    int vpac3_dual_fcp_enable;
    int num_frames_to_run;
    int sensor_sel;
    int num_viss_out_buf;
    bool is_interactive;
    bool test_mode;
    /*DCC read from FS. This will be appended to DCC from the driver*/
    uint8_t* fs_dcc_buf_viss;
    uint32_t fs_dcc_numbytes_viss;
    uint8_t* fs_dcc_buf_2a;
    uint32_t fs_dcc_numbytes_2a = 0;
    uint8_t* fs_dcc_buf_ldc;
    uint32_t fs_dcc_numbytes_ldc;
    char* sensor_name;
    int selectedCam;
    struct {
        int sensor_dcc_id;
        struct {
            int ee_mode;
            int mux_output0;
            int mux_output1;
            int mux_output2;
            int mux_output3;
            int mux_output4;
            int chroma_mode;
        } fcp[2];
        int h3a_in;
        int h3a_aewb_af_mode;
        int channel_id;
        int bypass_cac;
        int fcp1_config;
        int output_fcp_mapping[5];
        int enable_ctx;
        int bypass_glbce;
        int bypass_nsf4;
    } viss_params;
} AppObj;

#ifdef UNIT_TEST
    inline vx_uint32 write_output_image_fp(FILE *fp, vx_image out_nv12) {
        return (vx_uint32)mock().actualCall("write_output_image_fp").withParameter("fp", fp).withParameter("out_nv12",out_nv12).returnIntValue();
    }

    inline void app_set_cfg_default(AppObj *obj) {
        // Use mock support to track calls
        mock().actualCall("app_set_cfg_default").withParameter("obj", obj);
        // Simulate setting default configuration (do nothing)
    }

    inline void app_show_usage(int argc, char *argv[]) {
        mock().actualCall("app_show_usage").withParameter("argc", argc).withParameter("argv", argv);
    }

    inline void app_parse_cfg_file(AppObj *obj, const char *filename) {
        mock().actualCall("app_parse_cfg_file").withParameter("obj", obj).withParameter("filename", filename);
    }

    inline vx_image vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, int format) {
        return (vx_image)mock().actualCall("vxCreateImage")
            .withParameter("context", context)
            .withParameter("width", width)
            .withParameter("height", height)
            .withParameter("format", format)
            .returnPointerValue();
    }

    inline void tivx_vpac_viss_params_init(void* params) {
        mock().actualCall("tivx_vpac_viss_params_init").withParameter("params", params);
    }

    inline vx_user_data_object vxCreateUserDataObject(vx_context context, const vx_char* name, vx_size size, void* ptr) {
        return (vx_user_data_object)mock().actualCall("vxCreateUserDataObject")
            .withParameter("context", context)
            .withParameter("name", name)
            .withParameter("size", size)
            .withParameter("ptr", ptr)
            .returnPointerValue();
    }

    inline int32_t appIssGetDCCSize2A(char *sensor_name, uint32_t wdr_mode){
        return (int32_t)mock().actualCall("appIssGetDCCSize2A")
            .withParameter("sensor_name", sensor_name)
            .withParameter("name", wdr_mode)
            .returnIntValue();
    }

    inline int32_t appIssGetDCCBuff2A(char *sensor_name, uint32_t wdr_mode, uint8_t *dcc_buf, int32_t num_bytes){
        return (int32_t)mock().actualCall("appIssGetDCCBuff2A")
            .withParameter("sensor_name", sensor_name)
            .withParameter("wdr_mode", wdr_mode)
            .withParameter("dcc_buf", dcc_buf)
            .withParameter("num_bytes", num_bytes)
            .returnIntValue();
    }

    inline vx_status vxGetStatus(vx_reference reference){
        return (int32_t)mock().actualCall("vxGetStatus")
            .withParameter("reference", reference)
            .returnIntValue();
    }

    inline vx_node tivxAewbNode(vx_graph graph, vx_user_data_object configuration, vx_distribution histogram, vx_user_data_object h3a_aew_af, vx_user_data_object ae_awb_result_prev, vx_user_data_object ae_awb_result, vx_user_data_object dcc_param){
        return (vx_node)mock().actualCall("tivxAewbNode")
            .withParameter("graph", graph)
            .withParameter("configuration", configuration)
            .withParameter("histogram", histogram)
            .withParameter("h3a_aew_af", h3a_aew_af)
            .withParameter("ae_awb_result_prev", ae_awb_result_prev)
            .withParameter("ae_awb_result", ae_awb_result)
            .withParameter("dcc_param", dcc_param)
            .returnPointerValue();
    }

    inline vx_status vxSetNodeTarget(vx_node node, vx_enum target_enum, const char *target_string){
        return (vx_status)mock().actualCall("vxSetNodeTarget")
            .withParameter("node", node)
            .withParameter("target_enum", target_enum)
            .withParameter("target_string", target_string)
            .returnIntValue();
    }

    inline vx_status tivxSetNodeParameterNumBufByIndex(vx_node node, vx_uint32 idx, vx_uint32 num_buf){
        return (vx_status)mock().actualCall("tivxSetNodeParameterNumBufByIndex")
            .withParameter("node", node)
            .withParameter("idx", idx)
            .withParameter("num_buf", num_buf)
            .returnIntValue();      
    }

    inline vx_status vxMapUserDataObject(vx_user_data_object obj, vx_size offset, vx_size size, vx_map_id* map_id, void** ptr, int usage, int mem_type, int flags) {
        return (vx_status)mock().actualCall("vxMapUserDataObject")
            .withParameter("obj", obj)
            .withParameter("offset", offset)
            .withParameter("size", size)
            .withOutputParameter("map_id", map_id)
            .withOutputParameter("ptr", ptr)
            .withParameter("usage", usage)
            .withParameter("mem_type", mem_type)
            .withParameter("flags", flags)
            .returnIntValue();
    }

    inline vx_status vxUnmapUserDataObject(vx_user_data_object obj, vx_map_id map_id) {
        return (vx_status)mock().actualCall("vxUnmapUserDataObject")
            .withParameter("obj", obj)
            .withParameter("map_id", map_id)
            .returnIntValue();
    }

    inline int appIssGetDCCSizeVISS(const char* sensor_name, vx_uint32 sensor_wdr_enabled) {
        return mock().actualCall("appIssGetDCCSizeVISS")
            .withParameter("sensor_name", sensor_name)
            .withParameter("sensor_wdr_enabled", sensor_wdr_enabled)
            .returnIntValue();
    }

    inline vx_status appIssGetDCCBuffVISS(const char* sensor_name, vx_uint32 sensor_wdr_enabled, uint8_t* buffer, int32_t size) {
        return (vx_status)mock().actualCall("appIssGetDCCBuffVISS")
            .withParameter("sensor_name", sensor_name)
            .withParameter("sensor_wdr_enabled", sensor_wdr_enabled)
            .withParameter("buffer", buffer)
            .withParameter("size", size)
            .returnIntValue();
    }

    inline vx_node tivxVpacVissNode(vx_graph graph, vx_user_data_object configuration, vx_user_data_object ae_awb_result, vx_user_data_object dcc_param_viss, vx_image input_image, vx_image output0, vx_image output1, vx_image output2, vx_image output3, vx_image output4, vx_user_data_object h3a_aew_af, vx_user_data_object h3a_hist, vx_image output5, vx_image output6) {
        return (vx_node)mock().actualCall("tivxVpacVissNode")
            .withParameter("graph", graph)
            .withParameter("configuration", configuration)
            .withParameter("dcc_param_viss", dcc_param_viss)
            .withParameter("input_image", input_image)
            .withParameter("output0", output0)
            .withParameter("output1", output1)
            .withParameter("output2", output2)
            .withParameter("output3", output3)
            .withParameter("output4", output4)
            .withParameter("h3a_aew_af", h3a_aew_af)
            .withParameter("output5", output5)
            .withParameter("output6", output6)
            .returnPointerValue();
    }

    inline vx_status vxSetReferenceName(vx_reference ref, const char* name) {
        return (vx_status)mock().actualCall("vxSetReferenceName")
            .withParameter("ref", ref)
            .withParameter("name", name)
            .returnIntValue();
    }
    
    inline vx_status tivxCaptureRegisterErrorFrame(vx_node cap_node, vx_reference raw_img){
        return (vx_status)mock().actualCall("tivxCaptureRegisterErrorFrame")
            .withParameter("cap_node", cap_node)
            .withParameter("raw_img", raw_img).returnIntValue();

}

#endif  // UNIT_TEST

#endif  // MOCK_H