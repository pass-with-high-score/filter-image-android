#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <cmath>
#include <opencv2/opencv.hpp>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "native-lib::", __VA_ARGS__)
extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_grayscale(JNIEnv *env, jobject, jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    cv::Mat src(info.height, info.width, CV_8UC4, pixels);

    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);

    cv::cvtColor(gray, src, cv::COLOR_GRAY2RGBA);

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

    int radius = 5;
    int kernelSize = radius * 2 + 1;

    cv::Mat mat(info.height, info.width, CV_8UC4, pixels);

    cv::blur(mat, mat, cv::Size(kernelSize, kernelSize));

    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_invert(JNIEnv *env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    cv::Mat mat(info.height, info.width, CV_8UC4, pixels);

    // Lưu alpha channel
    cv::Mat alpha;
    cv::extractChannel(mat, alpha, 3);

    // Đảo toàn bộ
    cv::bitwise_not(mat, mat);

    // Khôi phục alpha
    cv::insertChannel(alpha, mat, 3);

    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_brightnessContrast(
        JNIEnv *env, jobject, jobject bitmap, jint brightness, jfloat contrast) {

    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    // Tạo cv::Mat từ bitmap
    cv::Mat mat(info.height, info.width, CV_8UC4, pixels);

    // Áp dụng brightness & contrast - OpenCV tự động clamp [0, 255]
    mat.convertTo(mat, -1, contrast, brightness);

    AndroidBitmap_unlockPixels(env, bitmap);
}


extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_vintage(JNIEnv *env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void *pixels;

    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    cv::Mat mat(info.height, info.width, CV_8UC4, pixels);
    cv::Mat matFloat;
    mat.convertTo(matFloat, CV_32FC4, 1.0/255.0); // Normalize [0,1]

    std::vector<cv::Mat> ch;
    cv::split(matFloat, ch);

    // Grayscale cho desaturation
    cv::Mat gray = 0.11f * ch[0] + 0.59f * ch[1] + 0.3f * ch[2];

    // Áp dụng tất cả effect
    ch[2] = (ch[2] * 0.85f + gray * 0.15f) * 0.9f + (15.0f + 15.0f)/255.0f; // R
    ch[1] = (ch[1] * 0.85f + gray * 0.15f) * 0.9f + 15.0f/255.0f;           // G
    ch[0] = (ch[0] * 0.85f + gray * 0.15f) * 0.9f + (15.0f - 10.0f)/255.0f; // B

    cv::merge(ch, matFloat);
    matFloat.convertTo(mat, CV_8UC4, 255.0); // Denormalize

    AndroidBitmap_unlockPixels(env, bitmap);
}

extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_removeBackground(
        JNIEnv *env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void* pixels = nullptr;

    // Lấy thông tin bitmap
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        return;
    }

    // Lock bitmap để có thể đọc/ghi pixels
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) {
        return;
    }

    // Chuyển bitmap Android sang OpenCV Mat
    cv::Mat src(info.height, info.width, CV_8UC4, pixels);

    // Tạo mat tạm để xử lý
    cv::Mat bgr, gray, alpha;

    // Chuyển RGBA sang BGR để xử lý
    cv::cvtColor(src, bgr, cv::COLOR_RGBA2BGR);

    // Chuyển sang grayscale
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);

    // Áp dụng thresholding để tạo alpha channel
    // Các pixel đen (< threshold) sẽ thành 0 (transparent)
    // Các pixel khác sẽ thành 255 (opaque)
    cv::threshold(gray, alpha, 10, 255, cv::THRESH_BINARY);

    // Tách các channels của src
    std::vector<cv::Mat> channels;
    cv::split(src, channels);

    // Thay alpha channel cũ bằng alpha channel mới
    channels[3] = alpha;

    // Merge lại thành RGBA
    cv::Mat result;
    cv::merge(channels, result);

    // Copy kết quả về bitmap gốc
    result.copyTo(src);

    // Unlock bitmap
    AndroidBitmap_unlockPixels(env, bitmap);
}

// Hàm bổ sung: Remove background với màu tùy chỉnh
extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_removeBackgroundColor(
        JNIEnv *env, jobject, jobject bitmap, jint r, jint g, jint b, jint tolerance) {

    AndroidBitmapInfo info;
    void* pixels = nullptr;

    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return;

    cv::Mat src(info.height, info.width, CV_8UC4, pixels);
    cv::Mat bgr, mask;

    // Chuyển sang BGR
    cv::cvtColor(src, bgr, cv::COLOR_RGBA2BGR);

    // Tạo màu cần remove
    cv::Scalar lowerBound(b - tolerance, g - tolerance, r - tolerance);
    cv::Scalar upperBound(b + tolerance, g + tolerance, r + tolerance);

    // Tạo mask: pixel trong khoảng màu = 0 (đen), ngoài khoảng = 255 (trắng)
    cv::inRange(bgr, lowerBound, upperBound, mask);

    // Đảo ngược mask (pixel cần giữ = 255, pixel cần xóa = 0)
    cv::bitwise_not(mask, mask);

    // Áp dụng mask vào alpha channel
    std::vector<cv::Mat> channels;
    cv::split(src, channels);
    channels[3] = mask;

    cv::Mat result;
    cv::merge(channels, result);
    result.copyTo(src);

    AndroidBitmap_unlockPixels(env, bitmap);
}

// Hàm remove background nâng cao với GrabCut (tự động detect foreground)
extern "C" JNIEXPORT void JNICALL
Java_com_pwhs_cnative_ImageProcessor_removeBackgroundAuto(
        JNIEnv *env, jobject, jobject bitmap) {

    AndroidBitmapInfo info;
    void* pixels = nullptr;

    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return;

    cv::Mat src(info.height, info.width, CV_8UC4, pixels);
    cv::Mat bgr;

    cv::cvtColor(src, bgr, cv::COLOR_RGBA2BGR);

    // Tạo mask cho GrabCut
    cv::Mat mask = cv::Mat::zeros(bgr.size(), CV_8UC1);

    // Định nghĩa rectangle chứa foreground (giữa ảnh, bỏ viền)
    int margin = 10;
    cv::Rect rect(margin, margin,
            info.width - 2 * margin,
            info.height - 2 * margin);

    cv::Mat bgdModel, fgdModel;

    // Áp dụng GrabCut
    cv::grabCut(bgr, mask, rect, bgdModel, fgdModel,
            5, cv::GC_INIT_WITH_RECT);

    // Tạo binary mask: foreground = 255, background = 0
    cv::Mat mask2 = (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD);
    mask2.convertTo(mask2, CV_8UC1, 255);

    // Làm mịn edges
    cv::GaussianBlur(mask2, mask2, cv::Size(5, 5), 0);

    // Áp dụng vào alpha channel
    std::vector<cv::Mat> channels;
    cv::split(src, channels);
    channels[3] = mask2;

    cv::Mat result;
    cv::merge(channels, result);
    result.copyTo(src);

    AndroidBitmap_unlockPixels(env, bitmap);
}