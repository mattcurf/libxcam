/*
 * cl_fisheye_handler.h - CL fisheye handler
 *
 *  Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#ifndef XCAM_CL_FISHEYE_HANDLER_H
#define XCAM_CL_FISHEYE_HANDLER_H

#include "xcam_utils.h"
#include "cl_image_handler.h"
#include "cl_geo_map_handler.h"

namespace XCam {

struct CLFisheyeInfo {
    float    center_x;
    float    center_y;
    float    wide_angle;
    float    radius;
    float    rotate_angle; // clockwise

    CLFisheyeInfo ();
    bool is_valid () const;
};

class CLFisheyeHandler;
class CLFisheye2GPSKernel
    : public CLImageKernel
{
public:
    explicit CLFisheye2GPSKernel (SmartPtr<CLContext> &context, SmartPtr<CLFisheyeHandler> &handler);

protected:
    virtual XCamReturn prepare_arguments (
        SmartPtr<DrmBoBuffer> &input, SmartPtr<DrmBoBuffer> &output,
        CLArgument args[], uint32_t &arg_count,
        CLWorkSize &work_size);

private:
    SmartPtr<CLFisheyeHandler>  _handler;
    float                       _input_y_size[2];
    float                       _out_center[2]; //width/height
    float                       _radian_per_pixel[2];
    CLFisheyeInfo               _fisheye_info;
};

class CLFisheyeHandler
    : public CLImageHandler
    , public GeoKernelParamCallback
{
    friend class CLFisheye2GPSKernel;
public:
    explicit CLFisheyeHandler (bool use_map);
    void set_output_size (uint32_t width, uint32_t height);
    void get_output_size (uint32_t &width, uint32_t &height) const;

    void set_dst_range (float longitude, float latitude);
    void get_dst_range (float &longitude, float &latitude) const;
    void set_fisheye_info (const CLFisheyeInfo &info);
    const CLFisheyeInfo &get_fisheye_info () const {
        return _fisheye_info;
    }

protected:
    // derived from CLImageHandler
    virtual XCamReturn prepare_buffer_pool_video_info (
        const VideoBufferInfo &input,
        VideoBufferInfo &output);
    virtual XCamReturn prepare_parameters (SmartPtr<DrmBoBuffer> &input, SmartPtr<DrmBoBuffer> &output);
    virtual XCamReturn execute_done (SmartPtr<DrmBoBuffer> &output);

    // derived from GeoKernelParamCallback
    virtual SmartPtr<CLImage> get_geo_input_image (CLNV12PlaneIdx index);
    virtual SmartPtr<CLImage> get_geo_output_image (CLNV12PlaneIdx index);
    virtual SmartPtr<CLImage> get_geo_map_table () {
        return _geo_table;
    }
    virtual void get_geo_equivalent_out_size (float &width, float &height);
    virtual void get_geo_pixel_out_size (float &width, float &height);

private:
    SmartPtr<CLImage> &get_input_image (CLNV12PlaneIdx index) {
        XCAM_ASSERT (index < CLNV12PlaneMax);
        return _input [index];
    }
    SmartPtr<CLImage> &get_output_image (CLNV12PlaneIdx index) {
        XCAM_ASSERT (index < CLNV12PlaneMax);
        return _output [index];
    }

    SmartPtr<CLImage> create_geo_table (uint32_t width, uint32_t height);
    XCamReturn generate_fisheye_table (
        uint32_t fisheye_width, uint32_t fisheye_height, const CLFisheyeInfo &fisheye_info);

    XCAM_DEAD_COPY (CLFisheyeHandler);

private:
    uint32_t                         _output_width;
    uint32_t                         _output_height;
    float                            _range_longitude;
    float                            _range_latitude;
    CLFisheyeInfo                    _fisheye_info;
    float                            _map_factor;
    bool                             _use_map;
    SmartPtr<CLImage>                _geo_table;
    SmartPtr<CLImage>                _input[CLNV12PlaneMax];
    SmartPtr<CLImage>                _output[CLNV12PlaneMax];
};

SmartPtr<CLImageHandler>
create_fisheye_handler (SmartPtr<CLContext> &context, bool use_map = false);

}

#endif //XCAM_CL_FISHEYE_HANDLER_H

