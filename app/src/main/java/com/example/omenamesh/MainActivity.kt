package com.example.omenamesh

import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.format.DateUtils
import android.view.View
import androidx.*
import androidx.appcompat.app.AppCompatActivity
import com.example.omenamesh.databinding.ActivityMainBinding
import com.example.omenamesh.ui.theme.InfoActivity
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.TimeUnit

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var serverExecutor: ExecutorService
    private lateinit var clientExecutor: ExecutorService

    private var socket: Socket? = null
    private var writer: PrintWriter? = null
    private var reader: BufferedReader? = null
    private lateinit var serverSocket: ServerSocket

    // For Timestamp formatters
    private val timeFormat = SimpleDateFormat("h:mm a", Locale.getDefault())
    private val monthDayFormat = SimpleDateFormat("MMM d", Locale.getDefault())
    private val fullDateFormat = SimpleDateFormat("MMM d, yyyy", Locale.getDefault())

    private val handler = Handler(Looper.getMainLooper())
    private var isServerRunning = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Initialize thread pools
        serverExecutor = Executors.newSingleThreadExecutor()
        clientExecutor = Executors.newSingleThreadExecutor()

        // The part where the server starts in background
        startServer()

        binding.sendButton.setOnClickListener {
            val message = binding.messageInput.text.toString()
            if (message.isNotEmpty()) {
                val timestamp = System.currentTimeMillis()
                sendMessage("$timestamp|$message")  // Include timestamp with message
                updateChat(message, true, timestamp)
                binding.messageInput.text.clear()
            }
        }

        binding.infoButton.setOnClickListener {
            startActivity(Intent(this, InfoActivity::class.java))
        }
    }

    private fun startServer() {
        serverExecutor.execute {
            try {
                serverSocket = ServerSocket(12345)
                isServerRunning = true
                while (isServerRunning) {
                    val clientSocket = serverSocket.accept()
                    val input = BufferedReader(InputStreamReader(clientSocket.getInputStream()))
                    val receivedMessage = input.readLine()
                    if (receivedMessage != null) {
                        val (timestampStr, actualMessage) = receivedMessage.split("|", limit = 2)
                        updateChat(actualMessage, false, timestampStr.toLong())
                    }

                }
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    private fun sendMessage(message: String) {
        clientExecutor.execute {
            try {
                if (socket?.isClosed != false || socket == null) {
                    socket = Socket("192.168.1.2", 12345) // Replace with target IP
                    writer = PrintWriter(socket?.getOutputStream())
                }
                writer?.println(message)
                writer?.flush()
            } catch (e: Exception) {
                runOnUiThread {
                    updateChat("Error: ${e.message}", true, System.currentTimeMillis())
                }
            }
        }
    }

    private fun formatTimestamp(timestamp: Long): String {
        return when {
            DateUtils.isToday(timestamp) -> timeFormat.format(Date(timestamp))
            DateUtils.isToday(timestamp + TimeUnit.DAYS.toMillis(1)) -> "Yesterday"
            SimpleDateFormat("yyyy", Locale.getDefault()).format(Date(timestamp)) ==
                    SimpleDateFormat("yyyy", Locale.getDefault()).format(Date()) ->
                monthDayFormat.format(Date(timestamp))
            else -> fullDateFormat.format(Date(timestamp))
        }
    }

    private fun updateChat(message: String, isLocalUser: Boolean, timestamp: Long) {
        val formattedTime = formatTimestamp(timestamp)
        handler.post {
            binding.messageDisplay.append(
                "\n${if (isLocalUser) "You" else "Other"} • $formattedTime:\n$message\n"
            )
            // Auto-scroll to bottom
            binding.scrollView.post {
                binding.scrollView.fullScroll(View.FOCUS_DOWN)
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        isServerRunning = false
        try {
            serverSocket.close()
            socket?.close()
            serverExecutor.shutdown()
            clientExecutor.shutdown()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}