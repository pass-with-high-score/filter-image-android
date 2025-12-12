#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <cmath>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "native-lib::", __VA_ARGS__)
extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_grayscale(JNIEnv *env, jobject, jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);

    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    auto *line = (uint32_t *) pixels;
    for (int y = 0; y < info.height; ++y) {
        for (int x = 0; x < info.width; ++x) {
            uint32_t pixel = line[x];

            uint32_t r = (pixel >> 16) & 0xFF;
            uint32_t g = (pixel >> 8) & 0xFF;
            uint32_t b = pixel & 0xFF;

            uint32_t gray = (299 * r + 587 * g + 114 * b) / 1000;

            line[x] = (0xFF << 24) | (gray << 16) | (gray << 8) | gray;
        }
        line = (uint32_t *) ((char *) line + info.stride);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_blur(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    int width = info.width;
    int height = info.height;
    int radius = 5; // Bán kính blur

    // Copy dữ liệu gốc
    uint32_t *original = new uint32_t[width * height];
    memcpy(original, pixels, width * height * sizeof(uint32_t));

    uint32_t *line = (uint32_t *) pixels;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0, count = 0;

            // Lấy trung bình các pixel xung quanh
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        uint32_t pixel = original[ny * width + nx];
                        r += (pixel >> 16) & 0xFF;
                        g += (pixel >> 8) & 0xFF;
                        b += pixel & 0xFF;
                        count++;
                    }
                }
            }

            r /= count;
            g /= count;
            b /= count;

            line[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
        line = (uint32_t *) ((char *) line + info.stride);
    }

    delete[] original;
    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_invert(JNIEnv *env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    auto *line = (uint32_t *) pixels;
    for (int y = 0; y < info.height; y++) {
        for (int x = 0; x < info.width; x++) {
            uint32_t pixel = line[x];

            int r = (pixel >> 16) & 0xFF;
            int g = (pixel >> 8) & 0xFF;
            int b = pixel & 0xFF;

            // Đảo màu
            r = 255 - r;
            g = 255 - g;
            b = 255 - b;

            line[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
        line = (uint32_t *) ((char *) line + info.stride);
    }

    AndroidBitmap_unlockPixels(env, bitmap);
}