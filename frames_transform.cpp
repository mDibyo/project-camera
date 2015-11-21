#include "frames_transform.h"

#include <string.h>
#include <math.h>


FramesInplacePairwiseAbsDiffTransformer::FramesInplacePairwiseAbsDiffTransformer(
    size_t width, size_t height, size_t bytes_per_pixel)
    : has_prev(false),
      prev_frame(new libfreenect2::Frame(width, height, bytes_per_pixel)) { }

FramesInplacePairwiseAbsDiffTransformer::~FramesInplacePairwiseAbsDiffTransformer() {
  delete prev_frame;
}

bool FramesInplacePairwiseAbsDiffTransformer::transform(libfreenect2::FrameMap &frames) {
  size_t size = prev_frame->width * prev_frame->height * prev_frame->bytes_per_pixel;
  unsigned char *new_frame_data = frames[libfreenect2::Frame::Color]->data;
  unsigned char new_frame_data_copy[size];
  memcpy(new_frame_data_copy, new_frame_data, size);

  if (has_prev) {
    int diff = 0;
    for (size_t i = 0; i < size; i++) {
      diff = new_frame_data[i] - prev_frame->data[i];
      if (diff < 0) {
        new_frame_data[i] = (unsigned char) (- diff);
      } else {
        new_frame_data[i] = (unsigned char) diff;
      }
    }
  }

  prev_frame->data = new_frame_data;
  has_prev = true;
  return true;
}


FramesInplaceMinThresholdTransformer::FramesInplaceMinThresholdTransformer(unsigned char threshold)
    : min_threshold(threshold) { }

bool FramesInplaceMinThresholdTransformer::transform(libfreenect2::FrameMap &frames) {
  libfreenect2::Frame *frame = frames[libfreenect2::Frame::Color];
  unsigned char *frame_data = frame->data;

  for (size_t i = 0; i < frame->width * frame->height * frame->bytes_per_pixel; i++) {
    if (frame_data[i] < min_threshold) {
      frame_data[i] = 0;
    }
  }

  return true;
}


FramesNewPairwiseDistanceTransformer::FramesNewPairwiseDistanceTransformer(
    size_t width, size_t height, size_t bytes_per_pixel)
    : has_prev(false),
      prev_frame(new libfreenect2::Frame(width, height, bytes_per_pixel)) { }

FramesNewPairwiseDistanceTransformer::~FramesNewPairwiseDistanceTransformer() {
  delete prev_frame;
}

bool FramesNewPairwiseDistanceTransformer::transform(libfreenect2::FrameMap &inputFrames,
                                                     libfreenect2::FrameMap &outputFrames) {
  unsigned char *new_frame_data = inputFrames[libfreenect2::Frame::Color]->data;

  if (has_prev) {
    unsigned char *prev_frame_data = prev_frame->data;
    libfreenect2::Frame *transformed_frame = new libfreenect2::Frame(prev_frame->width, prev_frame->height, 1);
    unsigned char transformed_frame_data[prev_frame->width * prev_frame->height];

    long sum, diff;
    int j;
    for (size_t i = 0; i < prev_frame->width * prev_frame->height; i++) {
      sum = 0;
      for (j = 0; j < prev_frame->bytes_per_pixel; j++) {
        diff = new_frame_data[i * prev_frame->bytes_per_pixel + j] -
            prev_frame_data[i * prev_frame->bytes_per_pixel + j];
        sum += diff * diff;
      }
      transformed_frame_data[i] = sqrt(sum);
    }
    transformed_frame->data = transformed_frame_data;
    outputFrames[libfreenect2::Frame::Color] = transformed_frame;
  }

  prev_frame->data = new_frame_data;
  has_prev = true;
  return true;
}