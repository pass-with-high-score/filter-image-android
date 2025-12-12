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
}