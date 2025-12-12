package com.pwhs.cnative

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.view.View
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import com.pwhs.cnative.databinding.ActivityMainBinding
import org.opencv.android.OpenCVLoader

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private val imageProcessor = ImageProcessor()

    private var originalBitmap: Bitmap? = null
    private var filteredBitmap: Bitmap? = null
    private var currentBitmap: Bitmap? = null


    private val pickImage = registerForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri ->
        uri?.let {
            val bitmap = BitmapFactory.decodeStream(
                contentResolver.openInputStream(it)
            )

            originalBitmap = bitmap.copy(Bitmap.Config.ARGB_8888, true)
            currentBitmap = originalBitmap?.copy(Bitmap.Config.ARGB_8888, true)

            binding.imageView.setImageBitmap(currentBitmap)
            binding.tvPlaceholder.visibility = View.GONE
            enableFilterButtons(true)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.toolbar.apply {
            title = "C++ Image Filters"
            setTitleTextColor(getColor(android.R.color.white))
        }

        // Source - https://stackoverflow.com/a
// Posted by Bonins, modified by community. See post 'Timeline' for change history
// Retrieved 2025-12-12, License - CC BY-SA 4.0

        val ocvLoaded = OpenCVLoader.initDebug()
        if (ocvLoaded) {
            Toast.makeText(this@MainActivity, "OpenCV loaded", Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this@MainActivity, "Unable to load OpenCV", Toast.LENGTH_SHORT).show();
        }

        setupButtons()
        enableFilterButtons(false)
    }

    private fun setupButtons() {
        binding.btnSelect.setOnClickListener {
            pickImage.launch("image/*")
        }

        binding.btnGrayscale.setOnClickListener {
            applyFixedFilter { imageProcessor.grayscale(it) }
        }

        binding.btnInvert.setOnClickListener {
            applyFixedFilter { imageProcessor.invert(it) }
        }

        binding.btnBlur.setOnClickListener {
            applyFixedFilter { imageProcessor.blur(it) }
        }

        binding.sliderBrightness.addOnChangeListener { _, _, _ ->
            applyBC()
        }

        binding.sliderContrast.addOnChangeListener { _, _, _ ->
            applyBC()
        }

        binding.btnVintage.setOnClickListener {
            applyFilter { imageProcessor.vintage(it) }
        }

        binding.btnReset.setOnClickListener {
            currentBitmap = originalBitmap?.copy(Bitmap.Config.ARGB_8888, true)
            binding.imageView.setImageBitmap(currentBitmap)
            binding.apply {
                // reset sliders
                sliderBrightness.value = 0f
                sliderContrast.value = 1f
            }
        }
    }

    /**
     * Gọi filter brightness + contrast realtime
     */
    private fun applyBC() {
        val base = filteredBitmap ?: return

        val bmp = base.copy(Bitmap.Config.ARGB_8888, true)

        val brightness = binding.sliderBrightness.value.toInt()
        val contrast = binding.sliderContrast.value

        imageProcessor.brightnessContrast(bmp, brightness, contrast)

        currentBitmap = bmp
        binding.imageView.setImageBitmap(bmp)
    }


    /**
     * Áp dụng filter 1 lần
     */
    private fun applyFilter(filter: (Bitmap) -> Unit) {
        val src = originalBitmap ?: return

        val bmp = src.copy(Bitmap.Config.ARGB_8888, true)
        filter(bmp)

        currentBitmap = bmp
        binding.imageView.setImageBitmap(bmp)
    }


    private fun applyFixedFilter(filter: (Bitmap) -> Unit) {
        val src = originalBitmap ?: return

        val bmp = src.copy(Bitmap.Config.ARGB_8888, true)
        filter(bmp)

        filteredBitmap = bmp
        applyBC()  // update preview
    }


    private fun enableFilterButtons(enabled: Boolean) {
        binding.btnGrayscale.isEnabled = enabled
        binding.btnInvert.isEnabled = enabled
        binding.btnBlur.isEnabled = enabled
        binding.btnVintage.isEnabled = enabled
        binding.btnReset.isEnabled = enabled
        binding.sliderBrightness.isEnabled = enabled
        binding.sliderContrast.isEnabled = enabled
    }
}
