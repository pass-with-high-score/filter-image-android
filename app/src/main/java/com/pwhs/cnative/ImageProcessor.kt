package com.pwhs.cnative

import android.graphics.Bitmap

class ImageProcessor {
    companion object {
        init {
            System.loadLibrary("cnative")
        }
    }

    external fun grayscale(bitmap: Bitmap)
    external fun invert(bitmap: Bitmap)
    external fun blur(bitmap: Bitmap)
    external fun brightnessContrast(bitmap: Bitmap, brightness: Int, contrast: Float)
    external fun vintage(bitmap: Bitmap)
    external fun removeBackground(bitmap: Bitmap)
    external fun removeBackgroundAuto(bitmap: Bitmap)
}