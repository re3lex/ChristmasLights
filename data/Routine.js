const modesMap = {
  "0": "blendwave",
  "1": "rainbow_beat",
  "2": "two_sin #1",
  "3": "one_sin_pal #1",
  "4": "noise8_pal #1",
  "5": "two_sin #2",
  "6": "one_sin_pal #2",
  "7": "juggle_pal #1",
  "8": "matrix_pal #1",
  "9": "two_sin #3",
  "10": "one_sin_pal #3",
  "11": "three_sin_pal #1",
  "12": "serendipitous_pal",
  "13": "one_sin_pal #4",
  "14": "two_sin #4",
  "15": "matrix_pal #2",
  "16": "noise8_pal #2",
  "17": "plasma(11, 23, 4, 18)",
  "18": "two_sin #5",
  "19": "rainbow_march #1",
  "20": "three_sin_pal #2",
  "21": "rainbow_march #2",
  "22": "noise16_pal #1",
  "23": "one_sin_pal #5",
  "24": "plasma(23, 15, 6, 7)",
  "25": "confetti_pal #1",
  "26": "two_sin #6",
  "27": "matrix_pal #3",
  "28": "one_sin_pal #6",
  "29": "confetti_pal #2",
  "30": "plasma(8, 7, 9, 13)",
  "31": "juggle_pal #2",
  "32": "one_sin_pal #7",
  "33": "three_sin_pal #3",
  "34": "rainbow_march #3",
  "35": "plasma(11, 17, 20, 23)",
  "36": "confetti_pal #3",
  "37": "noise16_pal #2",
  "38": "noise8_pal #3",
  "39": "fire()",
  "40": "candles()",
  "41": "colorwaves()",
};

class Routine {

  constructor() {
    this.socked;
    this.autoScroll = false;
  }

  start() {
    this.socket = new WebSocket('ws://' + window.location.hostname + '/ws');
    this.socket.onmessage = (evt) => this.onSocketMessage(evt);
    this.socket.onerror = (evt) => this.onSocketError(evt);
    this.socket.onclose = (evt) => this.onSocketClose(evt);

    const urlParams = new URLSearchParams(window.location.search);
    const isAdmin = urlParams.get('admin') == "1";

    if (isAdmin) {
      const autoScrollCbx = document.querySelector("#autoscrollCheckbox");
      autoScrollCbx.addEventListener('change', (e) => {
        this.autoScroll = e.target.checked;
        //autoscrollCheckbox
        if (this.autoScroll) {
          var textArea = document.getElementById('rxConsole');
          textArea.scrollTo(0, textArea.scrollHeight);
        }
      });
      $('#autoscrollCheckbox').click();

      $('#nav-bar').css({ "display": "" });
    }
  }

  onSocketError(evt) {
    console.error("WebSocket error observed:", evt);
    $('#alert_placeholder').html(`
    <div class="alert alert-danger" role="alert">
      WebSocket error!
    </div>
    `);
  }
  onSocketClose(evt) {
    console.error("WebSocket is closed:", evt);
    $('#alert_placeholder').html(`
    <div class="alert alert-danger" role="alert">
      WebSocket is closed!
    </div>
    `);
  }

  onSocketMessage(evt) {
    const ts = new Date().toLocaleDateString() + ' ' + new Date().toLocaleTimeString() + '\r\n';
    let json;
    try {
      json = evt.data ? JSON.parse(evt.data.replace(/\r/g, "\\r").replace(/\n/g, "\\n")) : {};
    } catch (error) {
      console.warn("error:", error);
      console.warn("Original message:", evt.data);
      return;
    }

    const textArea = document.getElementById('rxConsole');

    let consoleUpdated = false
    if (json.message) {
      textArea.value += ts + json.message + '\r\n\r\n';
      consoleUpdated = true;
    }

    if (json.data) {
      textArea.value += json.data;
      consoleUpdated = true;
    }
    if (this.autoScroll && consoleUpdated) {
      textArea.scrollTo(0, textArea.scrollHeight);
    }
    if (json.settings) {
      this.handleSettingsLoad(json.settings);
    }
  }

  handleSettingsLoad(settings) {
    const { my_modes = [], max_bright, current_mode, demo_mode, demo_duration, glitter = false } = settings;

    this.my_modes = my_modes;

    $('#max_bright').attr('value', max_bright);
    $('#demo_duration').attr('value', demo_duration);
    $('#demo_mode').val(demo_mode);
    $('#glitter').prop('checked', glitter);
    this.loadAllowedModes(demo_mode, current_mode);
  }

  loadAllowedModes(demo_mode, selectedMode) {
    const modesField = $("#my_modes");
    modesField.empty();

    let modes = Object.keys(modesMap);
    if (demo_mode == '3' || demo_mode == '4') {
      modes = this.my_modes;
    }

    modes.forEach(mode => {
      const modeName = modesMap[mode];
      const option = $("<option></option>").attr("value", mode).text(`${mode}. ${modeName}`);
      if (mode == selectedMode) {
        option.attr('selected', true);
      }
      modesField.append(option);
    });
  }

  onBrightnessChange(e) {
    const { value = 0, min, max } = e.target;

    let fixedValue = Math.max(parseInt(value), min);
    fixedValue = Math.min(fixedValue, max);

    const cmd = {
      cmd: 'MAX_BRIGHT',
      value: fixedValue
    }
    this.socket.send(JSON.stringify(cmd));
  }

  onDemoDurationChange(e) {
    const { value = 0, min } = e.target;

    let fixedValue = Math.max(parseInt(value), min);

    const cmd = {
      cmd: 'DEMO_DURATION',
      value: fixedValue
    }
    this.socket.send(JSON.stringify(cmd));
  }

  onModeChange(e) {
    const { value } = e.target;

    if (!value) {
      return;
    }
    const cmd = {
      cmd: 'CURRENT_MODE',
      value
    }
    this.socket.send(JSON.stringify(cmd));
  }

  onGlitterChange(e) {
    const { checked: value } = e.target;

    const cmd = {
      cmd: 'GLITTER',
      value
    }
    this.socket.send(JSON.stringify(cmd));
  }

  onDemoModeChange(e) {
    const { value } = e.target;

    if (!value) {
      return;
    }

    this.loadAllowedModes(value, $("#my_modes").val());

    const cmd = {
      cmd: 'DEMO_MODE',
      value
    }
    this.socket.send(JSON.stringify(cmd));
  }

  sendInput() {
    this.socket.send(document.getElementById('txbuff').value);
    document.getElementById('txbuff').value = '';
  }
  resetESP() {
    const cmd = {
      cmd: 'RESET_ESP',
    }
    this.socket.send(JSON.stringify(cmd));
  }

  previousMode() {
    const cmd = {
      cmd: 'PREVIOUS_MODE',
    }
    this.socket.send(JSON.stringify(cmd));
  }
  nextMode() {
    const cmd = {
      cmd: 'NEXT_MODE',
    }
    this.socket.send(JSON.stringify(cmd));
  }
}