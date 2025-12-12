package com.pwhs.cnative


import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import com.pwhs.cnative.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private var originalBitmap: Bitmap? = null
    private var currentBitmap: Bitmap? = null
    private val imageProcessor = ImageProcessor()

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
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.toolbar.title = "C++ Image Filters"

        setupButtons()
        enableFilterButtons(false)
    }

    private fun setupButtons() {
        binding.btnSelect.setOnClickListener {
            pickImage.launch("image/*")
        }

        binding.btnGrayscale.setOnClickListener {
            applyFilter { imageProcessor.grayscale(it) }
        }

        binding.btnInvert.setOnClickListener {
            applyFilter { imageProcessor.invert(it) }
        }

        binding.btnBlur.setOnClickListener {
            applyFilter { imageProcessor.blur(it) }
        }

        binding.btnReset.setOnClickListener {
            currentBitmap = originalBitmap?.copy(Bitmap.Config.ARGB_8888, true)
            binding.imageView.setImageBitmap(currentBitmap)
        }
    }

    private fun applyFilter(filter: (Bitmap) -> Unit) {
        currentBitmap = originalBitmap?.copy(Bitmap.Config.ARGB_8888, true)
        currentBitmap?.let {
            filter(it)
            binding.imageView.setImageBitmap(it)
        }
    }

    private fun enableFilterButtons(enabled: Boolean) {
        binding.btnGrayscale.isEnabled = enabled
        binding.btnInvert.isEnabled = enabled
        binding.btnBlur.isEnabled = enabled
        binding.btnReset.isEnabled = enabled
    }
}