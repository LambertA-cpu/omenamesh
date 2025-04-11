package com.example.omenamesh.ui.theme

import android.Manifest
import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.net.Network
//import android.net.NetworkInfo
import android.net.wifi.WifiManager
import android.net.wifi.p2p.WifiP2pConfig
import android.net.wifi.p2p.WifiP2pDevice
import android.net.wifi.p2p.WifiP2pDeviceList
import android.net.wifi.p2p.WifiP2pInfo
import android.net.wifi.p2p.WifiP2pManager
import android.net.wifi.p2p.WifiP2pManager.Channel
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresPermission
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.omenamesh.R
import com.example.omenamesh.databinding.ActivityInfoBinding
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.IOException
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.Socket
import java.net.SocketTimeoutException
//import kotlin.math.*
import kotlin.math.pow
import kotlin.math.roundToInt
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.os.Build
import androidx.annotation.RequiresApi

class InfoActivity : AppCompatActivity() {
    private lateinit var binding: ActivityInfoBinding
    private var measurementJob: Job? = null
    private var isMeasuring = false
    private var selectedDevice: WifiP2pDevice? = null
    private var peerIpAddress: String? = null

    // Wi-Fi Direct variables
    private lateinit var manager: WifiP2pManager
    private lateinit var channel: Channel
    private lateinit var receiver: BroadcastReceiver
    private val peers = mutableListOf<WifiP2pDevice>()
    private lateinit var peersAdapter: PeersAdapter

// Permissions
    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        if (permissions.all { it.value }) {
            discoverPeers()
        } else {
            Toast.makeText(this, "Permissions required for Wi-Fi Direct", Toast.LENGTH_SHORT).show()
        }
    }

    // Network callback for real-time monitoring
    private val networkCallback = object : ConnectivityManager.NetworkCallback() {
        override fun onAvailable(network: Network) {
            runOnUiThread {
                updateConnectionStatus("Connected")
                binding.measurementToggle.isEnabled = true
            }
        }

        override fun onLost(network: Network) {
            runOnUiThread {
                updateConnectionStatus("Disconnected")
                binding.measurementToggle.isEnabled = false
                stopMeasurements()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityInfoBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupWifiDirect()
        setupUiListeners()
        setupPeersList()
        checkPermissions()

        // Register network callback
        val connectivityManager = getSystemService(CONNECTIVITY_SERVICE) as ConnectivityManager
        connectivityManager.registerNetworkCallback(
              NetworkRequest.Builder()
                .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .build(),
            networkCallback
        )
    }

    override fun onDestroy() {
        super.onDestroy()
        // Unregister network callback
        val connectivityManager = getSystemService(CONNECTIVITY_SERVICE) as ConnectivityManager
        connectivityManager.unregisterNetworkCallback(networkCallback)
    }

    override fun onResume() {
        super.onResume()
        registerReceiver(receiver, IntentFilter().apply {
            addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION)
        })
    }

    override fun onPause() {
        super.onPause()
        unregisterReceiver(receiver)
        stopMeasurements()
    }

    private fun setupWifiDirect() {
        manager = getSystemService(Context.WIFI_P2P_SERVICE) as WifiP2pManager
        channel = manager.initialize(this, mainLooper, null)

        receiver = object : BroadcastReceiver() {
            @RequiresApi(Build.VERSION_CODES.O)
            @SuppressLint("ObsoleteSdkInt")
            @RequiresPermission(allOf = [
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.NEARBY_WIFI_DEVICES
            ])
            override fun onReceive(context: Context, intent: Intent) {
                when (intent.action) {
                    WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION -> {
                        val state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1)
                        if (state == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
                            Log.d("WifiDirect", "Wi-Fi Direct is enabled")
                        } else {
                            Toast.makeText(context, "Wi-Fi Direct is disabled", Toast.LENGTH_SHORT).show()
                        }
                    }
                    WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION -> {
                        manager.requestPeers(channel, ::handlePeers)
                    }
                    WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION -> {
                        val connectivityManager = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager

                        val isWifiDirectConnected = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                            val activeNetwork = connectivityManager.activeNetwork
                            val caps = connectivityManager.getNetworkCapabilities(activeNetwork)
                            caps?.run {
                                hasCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET) &&
                                        (hasTransport(NetworkCapabilities.TRANSPORT_WIFI) ||
                                                hasTransport(NetworkCapabilities.TRANSPORT_WIFI_AWARE))
                            } ?: false
                        } else {
                            @Suppress("DEPRECATION")
                            connectivityManager.activeNetworkInfo?.run {
                                isConnected && type == ConnectivityManager.TYPE_WIFI
                            } ?: false
                        }

                        if (isWifiDirectConnected) {
                            manager.requestConnectionInfo(channel, ::handleConnection)
                        } else {
                            updateConnectionStatus("Disconnected")
                            binding.measurementToggle.isEnabled = false
                            stopMeasurements()
                        }
                    }
                    WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION -> {
                        // Device details changed
                    }
                }
            }
        }
    }
    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.apply {
            setDisplayHomeAsUpEnabled(true)
            title = "Wi-Fi Direct Measurements"
        }
        binding.toolbar.setNavigationOnClickListener { finish() }
    }

    private fun setupUiListeners() {
        binding.discoverButton.setOnClickListener {
            checkPermissions()
        }

        binding.connectButton.setOnClickListener {
            selectedDevice?.let { device ->
                connectToDevice(device)
            }
        }

        binding.measurementToggle.setOnClickListener {
            if (isMeasuring) {
                stopMeasurements()
            } else {
                startMeasurements()
            }
        }
    }

    private fun setupPeersList() {
        peersAdapter = PeersAdapter(peers) { device ->
            selectedDevice = device
            binding.connectButton.isEnabled = true
        }

        binding.deviceList.apply {
            layoutManager = LinearLayoutManager(this@InfoActivity)
            adapter = peersAdapter
        }
    }

    private fun checkPermissions() {
        val requiredPermissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            arrayOf(
                Manifest.permission.ACCESS_FINE_LOCATION,
                Manifest.permission.ACCESS_WIFI_STATE,
                Manifest.permission.CHANGE_WIFI_STATE,
                Manifest.permission.NEARBY_WIFI_DEVICES,
                Manifest.permission.ACCESS_NETWORK_STATE
            )
        } else {
            TODO("VERSION.SDK_INT < TIRAMISU")
        }

        if (requiredPermissions.all {
                ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
            }) {
            discoverPeers()
        } else {
            requestPermissionLauncher.launch(requiredPermissions)
        }
    }

    @SuppressLint("InlinedApi")
    private fun discoverPeers() {
        binding.progressBar.visibility = View.VISIBLE
        binding.deviceList.visibility = View.VISIBLE
        binding.metricsScrollView.visibility = View.GONE

        val requiredPermissions = arrayOf(
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.NEARBY_WIFI_DEVICES
        )

        if (requiredPermissions.all {
                ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
            }) {
            try {
                manager.discoverPeers(channel, object : WifiP2pManager.ActionListener {
                    override fun onSuccess() {
                        Log.d("WifiDirect", "Discovery started")
                    }

                    override fun onFailure(reason: Int) {
                        runOnUiThread {
                            binding.progressBar.visibility = View.GONE
                            when (reason) {
                                WifiP2pManager.P2P_UNSUPPORTED ->
                                    Toast.makeText(this@InfoActivity, "Wi-Fi Direct not supported", Toast.LENGTH_SHORT).show()
                                WifiP2pManager.BUSY ->
                                    Toast.makeText(this@InfoActivity, "System busy with Wi-Fi Direct", Toast.LENGTH_SHORT).show()
                                else ->
                                    Toast.makeText(this@InfoActivity, "Discovery failed: $reason", Toast.LENGTH_SHORT).show()
                            }
                        }
                    }
                })
            } catch (e: SecurityException) {
                runOnUiThread {
                    binding.progressBar.visibility = View.GONE
                    Toast.makeText(
                        this@InfoActivity,
                        "Permission denied for Wi-Fi Direct discovery",
                        Toast.LENGTH_SHORT
                    ).show()
                    Log.e("WifiDirect", "SecurityException", e)
                }
            }
        } else {
            binding.progressBar.visibility = View.GONE
            requestPermissionLauncher.launch(requiredPermissions)
        }
    }

    @SuppressLint("NotifyDataSetChanged")
    private fun handlePeers(peerList: WifiP2pDeviceList) {
        runOnUiThread {
            binding.progressBar.visibility = View.GONE
            peers.clear()
            peers.addAll(peerList.deviceList)
            peersAdapter.notifyDataSetChanged()

            if (peers.isEmpty()) {
                Toast.makeText(this@InfoActivity, "No devices found", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(this@InfoActivity, "${peers.size} device(s) found", Toast.LENGTH_SHORT).show()
            }
        }
    }

    private fun connectToDevice(device: WifiP2pDevice) {
        val requiredPermissions = arrayOf(
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.NEARBY_WIFI_DEVICES
        )

        if (requiredPermissions.all {
                ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
            }) {
            binding.progressBar.visibility = View.VISIBLE

            val config = WifiP2pConfig().apply {
                deviceAddress = device.deviceAddress
                // Additional configuration if needed
                groupOwnerIntent = 0 // 0-15 where 15 has highest chance of becoming group owner
            }

            try {
                manager.connect(channel, config, object : WifiP2pManager.ActionListener {
                    override fun onSuccess() {
                        Log.d("WifiDirect", "Connection initiated")
                    }

                    override fun onFailure(reason: Int) {
                        runOnUiThread {
                            binding.progressBar.visibility = View.GONE
                            when (reason) {
                                WifiP2pManager.ERROR ->
                                    Toast.makeText(this@InfoActivity, "Connection error occurred", Toast.LENGTH_SHORT).show()
                                WifiP2pManager.P2P_UNSUPPORTED ->
                                    Toast.makeText(this@InfoActivity, "Wi-Fi Direct not supported", Toast.LENGTH_SHORT).show()
                                WifiP2pManager.BUSY ->
                                    Toast.makeText(this@InfoActivity, "System busy with Wi-Fi Direct", Toast.LENGTH_SHORT).show()
                                else ->
                                    Toast.makeText(this@InfoActivity, "Connection failed: $reason", Toast.LENGTH_SHORT).show()
                            }
                        }
                    }
                })
            } catch (e: SecurityException) {
                runOnUiThread {
                    binding.progressBar.visibility = View.GONE
                    Toast.makeText(
                        this@InfoActivity,
                        "Permission denied for Wi-Fi Direct connection",
                        Toast.LENGTH_SHORT
                    ).show()
                    Log.e("WifiDirect", "SecurityException", e)
                }
            }
        } else {
            requestPermissionLauncher.launch(requiredPermissions)
        }
    }

    private fun handleConnection(info: WifiP2pInfo) {
        binding.progressBar.visibility = View.GONE
        if (info.groupFormed && info.isGroupOwner) {
            peerIpAddress = "192.168.49.1" // Group owner IP
            updateConnectionStatus("Connected as Group Owner")
        } else if (info.groupFormed) {
            peerIpAddress = info.groupOwnerAddress.hostAddress
            updateConnectionStatus("Connected to Group Owner")
        }

        binding.measurementToggle.isEnabled = true
        binding.metricsScrollView.visibility = View.VISIBLE
        binding.deviceList.visibility = View.GONE
    }

    private fun updateConnectionStatus(status: String) {
        binding.connectionStatusText.text = status
    }

    private fun startMeasurements() {
        if (peerIpAddress == null) return

        isMeasuring = true
        binding.measurementToggle.text = "Stop Measurements"
        binding.measurementToggle.setIconResource(R.drawable.ic_stop)

        measurementJob = CoroutineScope(Dispatchers.IO).launch {
            var packetCount = 0
            var lostPacketCount = 0
            val packetBuffer = ByteArray(32) { it.toByte() }

            while (isActive && isMeasuring) {
                try {
                    // Measure signal strength
                    val wifiManager = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
                    val rssi = wifiManager.connectionInfo.rssi
                    val distance = calculateDistanceFromRssi(rssi)

                    // Measure latency
                    val latency = measureLatency(peerIpAddress!!)

                    // Measure packet loss
                    val packetLoss = if (packetCount > 0) {
                        (lostPacketCount.toDouble() / packetCount) * 100
                    } else {
                        0.0
                    }

                    // Send test packet (for packet loss calculation)
                    try {
                        DatagramSocket().use { socket ->
                            val address = InetAddress.getByName(peerIpAddress)
                            val packet = DatagramPacket(packetBuffer, packetBuffer.size, address, 8888)
                            socket.send(packet)
                            packetCount++
                        }
                    } catch (e: IOException) {
                        lostPacketCount++
                    }

                    // Update UI
                    withContext(Dispatchers.Main) {
                        binding.signalStrengthText.text = "$rssi dBm"
                        "${latency.roundToInt()} ms".also { binding.latencyText.text = it }
                        binding.packetLossText.text = "%.1f%%".format(packetLoss)
                        binding.distanceText.text = "%.1f m".format(distance)

                        // Set text colors based on quality
                        setTextColorBasedOnQuality(binding.latencyText, latency, 50.0, 100.0)
                        setTextColorBasedOnQuality(binding.packetLossText, packetLoss, 5.0, 20.0)
                        setTextColorBasedOnQuality(binding.signalStrengthText, rssi.toDouble(), -60.0, -80.0)
                    }

                    delay(1000) // Update every second
                } catch (e: Exception) {
                    Log.e("Measurements", "Error in measurement loop", e)
                }
            }
        }
    }

    private fun stopMeasurements() {
        isMeasuring = false
        measurementJob?.cancel()
        "Start Measurements".also { binding.measurementToggle.text = it }
        binding.measurementToggle.setIconResource(R.drawable.ic_play)
    }

    private suspend fun measureLatency(targetIp: String): Double {
        return withContext(Dispatchers.IO) {
            try {
                Socket().use { socket ->
                    val startTime = System.nanoTime()
                    socket.connect(InetSocketAddress(targetIp, 8888), 1000)
                    (System.nanoTime() - startTime) / 1_000_000.0
                }
            } catch (e: SocketTimeoutException) {
                1000.0 // Max timeout value
            } catch (e: Exception) {
                0.0
            }
        }
    }

    private fun calculateDistanceFromRssi(rssi: Int): Double {
        // Free space path loss model (simplified)
        val txPower = -50 // Calibrated TX power at 1 meter
        if (rssi >= txPower) return 0.1

        val ratio = (txPower - rssi) / 20.0
        return 10.0.pow(ratio)
    }

    private fun setTextColorBasedOnQuality(
        textView: TextView,
        value: Double,
        goodThreshold: Double,
        badThreshold: Double
    ) {
        val colorRes = when {
            value < goodThreshold -> R.color.good_quality
            value < badThreshold -> R.color.medium_quality
            else -> R.color.bad_quality
        }
        textView.setTextColor(ContextCompat.getColor(this, colorRes))
    }

    // Peers Adapter for device list
    private inner class PeersAdapter(
        private val devices: List<WifiP2pDevice>,
        private val onDeviceSelected: (WifiP2pDevice) -> Unit
    ) : RecyclerView.Adapter<PeersAdapter.DeviceViewHolder>() {

        inner class DeviceViewHolder(view: View) : RecyclerView.ViewHolder(view) {
            val deviceName: TextView = view.findViewById(R.id.deviceName)
            val deviceStatus: TextView = view.findViewById(R.id.deviceStatus)
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): DeviceViewHolder {
            val view = LayoutInflater.from(parent.context)
                .inflate(R.layout.item_device, parent, false)
            return DeviceViewHolder(view)
        }

        override fun onBindViewHolder(holder: DeviceViewHolder, position: Int) {
            val device = devices[position]
            holder.deviceName.text = device.deviceName ?: "Unknown"
            holder.deviceStatus.text = when (device.status) {
                WifiP2pDevice.AVAILABLE -> "Available"
                WifiP2pDevice.INVITED -> "Invited"
                WifiP2pDevice.CONNECTED -> "Connected"
                WifiP2pDevice.FAILED -> "Failed"
                WifiP2pDevice.UNAVAILABLE -> "Unavailable"
                else -> "Unknown"
            }

            holder.itemView.setOnClickListener {
                onDeviceSelected(device)
            }
        }

        override fun getItemCount() = devices.size
    }
}