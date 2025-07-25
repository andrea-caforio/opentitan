// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

use crate::bootstrap::BootstrapOptions;
use crate::io::emu::{EmuState, EmuValue};
use crate::io::gpio::{
    ClockNature, MonitoringReadResponse, MonitoringStartResponse, PinMode, PullMode,
};
use crate::io::i2c::DeviceStatus;
use crate::io::spi::{MaxSizes, TransferMode};
use crate::io::uart::{FlowControl, Parity};
use crate::proxy::errors::SerializedError;
use crate::transport::Capabilities;
use crate::util::voltage::Voltage;

#[derive(Serialize, Deserialize)]
pub enum Message {
    // Request/response pairs.  There is no explicit identifier to link a response to a request,
    // as requests are processed and responses generated in the order they are received.
    Req(Request),
    Res(Result<Response, SerializedError>),
    // An "asynchronos message" is one that is not a direct response to a request, but can be sent
    // at any time, as part of a communication "channel" previously set up.
    Async { channel: u32, msg: AsyncMessage },
}

#[derive(Serialize, Deserialize)]
pub enum AsyncMessage {
    UartData { data: Vec<u8> },
}

#[derive(Serialize, Deserialize)]
pub enum Request {
    GetCapabilities,
    ApplyDefaultConfiguration,
    Gpio { id: String, command: GpioRequest },
    GpioMonitoring { command: GpioMonRequest },
    GpioBitbanging { command: GpioBitRequest },
    GpioDacBanging { command: GpioDacRequest },
    Uart { id: String, command: UartRequest },
    Spi { id: String, command: SpiRequest },
    I2c { id: String, command: I2cRequest },
    Emu { command: EmuRequest },
    Proxy(ProxyRequest),
}

#[derive(Serialize, Deserialize)]
pub enum Response {
    GetCapabilities(Capabilities),
    ApplyDefaultConfiguration,
    Gpio(GpioResponse),
    GpioMonitoring(GpioMonResponse),
    GpioBitbanging(GpioBitResponse),
    GpioDacBanging(GpioDacResponse),
    Uart(UartResponse),
    Spi(SpiResponse),
    I2c(I2cResponse),
    Emu(EmuResponse),
    Proxy(ProxyResponse),
}

#[derive(Serialize, Deserialize)]
pub enum GpioRequest {
    Write {
        logic: bool,
    },
    Read,
    SetMode {
        mode: PinMode,
    },
    SetPullMode {
        pull: PullMode,
    },
    AnalogWrite {
        value: f32,
    },
    AnalogRead,
    MultiSet {
        mode: Option<PinMode>,
        value: Option<bool>,
        pull: Option<PullMode>,
        analog_value: Option<f32>,
    },
}

#[derive(Serialize, Deserialize)]
pub enum GpioResponse {
    Write,
    Read { value: bool },
    SetMode,
    SetPullMode,
    AnalogWrite,
    AnalogRead { value: f32 },
    MultiSet,
}

#[derive(Serialize, Deserialize)]
pub enum GpioMonRequest {
    GetClockNature,
    Start {
        pins: Vec<String>,
    },
    Read {
        pins: Vec<String>,
        continue_monitoring: bool,
    },
}

#[derive(Serialize, Deserialize)]
pub enum GpioMonResponse {
    GetClockNature { resp: ClockNature },
    Start { resp: MonitoringStartResponse },
    Read { resp: MonitoringReadResponse },
}

#[derive(Serialize, Deserialize)]
pub enum BitbangEntryRequest {
    Write { data: Vec<u8> },
    Both { data: Vec<u8> },
    Delay { clock_ticks: u32 },
    Await { mask: u8, pattern: u8 },
}

#[derive(Serialize, Deserialize)]
pub enum BitbangEntryResponse {
    Write,
    Both { data: Vec<u8> },
    Delay,
    Await,
}

#[derive(Serialize, Deserialize)]
pub enum GpioBitRequest {
    Start {
        pins: Vec<String>,
        clock_ns: u64,
        entries: Vec<BitbangEntryRequest>,
    },
    Query,
}

#[derive(Serialize, Deserialize)]
pub enum GpioBitResponse {
    Start,
    // GpioBitRequest::Query will be answered by either QueryNotDone or QueryDone.
    QueryNotDone,
    QueryDone { entries: Vec<BitbangEntryResponse> },
}

#[derive(Serialize, Deserialize)]
pub enum DacBangEntryRequest {
    Write { data: Vec<f32> },
    Delay { clock_ticks: u32 },
    Linear { clock_ticks: u32 },
}

#[derive(Serialize, Deserialize)]
pub enum GpioDacRequest {
    Start {
        pins: Vec<String>,
        clock_ns: u64,
        entries: Vec<DacBangEntryRequest>,
    },
    Query,
}

#[derive(Serialize, Deserialize)]
pub enum GpioDacResponse {
    Start,
    // GpioDacRequest::Query will be answered by either QueryNotDone or QueryDone.
    QueryNotDone,
    QueryDone,
}

#[derive(Serialize, Deserialize)]
pub enum UartRequest {
    GetBaudrate,
    SetBaudrate {
        rate: u32,
    },
    SetBreak(bool),
    GetParity,
    SetParity(Parity),
    GetFlowControl,
    SetFlowControl(bool),
    GetDevicePath,
    Read {
        timeout_millis: Option<u32>,
        len: u32,
    },
    Write {
        data: Vec<u8>,
    },
    SupportsNonblockingRead,
    RegisterNonblockingRead,
}

#[derive(Serialize, Deserialize)]
pub enum UartResponse {
    GetBaudrate { rate: u32 },
    SetBaudrate,
    SetBreak,
    GetParity { parity: Parity },
    SetParity,
    GetFlowControl { flow_control: FlowControl },
    SetFlowControl,
    GetDevicePath { path: String },
    Read { data: Vec<u8> },
    Write,
    SupportsNonblockingRead { has_support: bool },
    RegisterNonblockingRead { channel: u32 },
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferRequest {
    Read { len: u32 },
    Write { data: Vec<u8> },
    Both { data: Vec<u8> },
    TpmPoll,
    GscReady,
}

#[derive(Serialize, Deserialize)]
pub enum SpiTransferResponse {
    Read { data: Vec<u8> },
    Write,
    Both { data: Vec<u8> },
    TpmPoll,
    GscReady,
}

#[derive(Serialize, Deserialize)]
pub enum SpiRequest {
    GetTransferMode,
    SetTransferMode {
        mode: TransferMode,
    },
    GetBitsPerWord,
    SetBitsPerWord {
        bits_per_word: u32,
    },
    GetMaxSpeed,
    SetMaxSpeed {
        value: u32,
    },
    SupportsBidirectionalTransfer,
    SupportsTpmPoll,
    SetPins {
        serial_clock: Option<String>,
        host_out_device_in: Option<String>,
        host_in_device_out: Option<String>,
        chip_select: Option<String>,
        gsc_ready: Option<String>,
    },
    GetMaxTransferCount,
    GetMaxTransferSizes,
    GetEepromMaxTransferSizes,
    SetVoltage {
        voltage: Voltage,
    },
    GetFlashromArgs,
    RunTransaction {
        transaction: Vec<SpiTransferRequest>,
    },
    AssertChipSelect,
    DeassertChipSelect,
}

#[derive(Serialize, Deserialize)]
pub enum SpiResponse {
    GetTransferMode {
        mode: TransferMode,
    },
    SetTransferMode,
    GetBitsPerWord {
        bits_per_word: u32,
    },
    SetBitsPerWord,
    GetMaxSpeed {
        speed: u32,
    },
    SetMaxSpeed,
    SupportsBidirectionalTransfer {
        has_support: bool,
    },
    SupportsTpmPoll {
        has_support: bool,
    },
    SetPins,
    GetMaxTransferCount {
        number: usize,
    },
    GetMaxTransferSizes {
        sizes: MaxSizes,
    },
    GetEepromMaxTransferSizes {
        sizes: MaxSizes,
    },
    SetVoltage,
    GetFlashromArgs {
        programmer: String,
    },
    RunTransaction {
        transaction: Vec<SpiTransferResponse>,
    },
    AssertChipSelect,
    DeassertChipSelect,
}

#[derive(Serialize, Deserialize)]
pub enum I2cTransferRequest {
    Read { len: u32 },
    Write { data: Vec<u8> },
    GscReady,
}

#[derive(Serialize, Deserialize)]
pub enum I2cTransferResponse {
    Read { data: Vec<u8> },
    Write,
    GscReady,
}

#[derive(Serialize, Deserialize)]
pub enum I2cRequest {
    SetModeHost,
    SetModeDevice {
        addr: u8,
    },
    GetMaxSpeed,
    SetMaxSpeed {
        value: u32,
    },
    SetPins {
        serial_clock: Option<String>,
        serial_data: Option<String>,
        gsc_ready: Option<String>,
    },
    RunTransaction {
        address: Option<u8>,
        transaction: Vec<I2cTransferRequest>,
    },
    GetDeviceStatus {
        timeout_millis: u32,
    },
    PrepareReadData {
        data: Vec<u8>,
        sticky: bool,
    },
}

#[derive(Serialize, Deserialize)]
pub enum I2cResponse {
    SetModeHost,
    SetModeDevice,
    GetMaxSpeed {
        speed: u32,
    },
    SetMaxSpeed,
    SetPins,
    RunTransaction {
        transaction: Vec<I2cTransferResponse>,
    },
    GetDeviceStatus {
        status: DeviceStatus,
    },
    PrepareReadData,
}

#[derive(Serialize, Deserialize)]
pub enum EmuRequest {
    GetState,
    Start {
        factory_reset: bool,
        args: HashMap<String, EmuValue>,
    },
    Stop,
}

#[derive(Serialize, Deserialize)]
pub enum EmuResponse {
    GetState { state: EmuState },
    Start,
    Stop,
}

#[derive(Serialize, Deserialize)]
pub enum ProxyRequest {
    Provides,
    Bootstrap {
        options: BootstrapOptions,
        payload: Vec<u8>,
    },
    ApplyPinStrapping {
        strapping_name: String,
    },
    RemovePinStrapping {
        strapping_name: String,
    },
    ApplyDefaultConfigurationWithStrapping {
        strapping_name: String,
    },
}

#[derive(Serialize, Deserialize)]
pub enum ProxyResponse {
    Provides {
        provides_map: HashMap<String, String>,
    },
    Bootstrap,
    ApplyPinStrapping,
    RemovePinStrapping,
    ApplyDefaultConfigurationWithStrapping,
}
