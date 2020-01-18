const char index_page[] PROGMEM = R"=====(


<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css" integrity="sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm" crossorigin="anonymous">
  <script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js" integrity="sha384-ApNbgh9B+Y1QKtv3Rn7W3mgPxhU9K/ScQsAP7hUibX39j7fakFPskvXusvfa0b4Q" crossorigin="anonymous"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js" integrity="sha384-JZR6Spejh4U02d8jOt6vLEHfe/JQGiRRSQQxSfFWpi1MquVdAyjUar5+76PVCmYl" crossorigin="anonymous"></script>


<style>
    textarea.form-control {
      height: 40vh !important;
      resize: none;
    }
</style>

<script>
  String.prototype.format = function(placeholders) {
      var s = this;
      for(var propertyName in placeholders) {
          var re = new RegExp('{' + propertyName + '}', 'gm');
          s = s.replace(re, placeholders[propertyName]);
      }    
      return s;
  };


  var Socket;
  var autoScroll = false;
  function start() {
    Socket = new WebSocket('ws://' + window.location.hostname + '/ws');
    
    Socket.onmessage = function(evt) {
      var ts = new Date().toLocaleDateString() + ' ' +new Date().toLocaleTimeString()+'\r\n';
      try {
        var json = evt.data ? JSON.parse(evt.data.replace(/\r/g,"\\r").replace(/\n/g,"\\n")) : {};
      }catch(error) {
        console.warn("error:", error);
        console.warn("Original message:", evt.data);
        return;
      }

      var textArea = document.getElementById('rxConsole');

      if(json.message) {
        textArea.value += ts + json.message+'\r\n\r\n';
      }

      if(json.data) {
        var data = json.data;
        textArea.value += data;
      }
      if(autoScroll) {
        textArea.scrollTo(0, textArea.scrollHeight);
      }
    }

    var autoScrollCbx = document.querySelector("#autoscrollCheckbox");
    autoScrollCbx.addEventListener( 'change', onAutoscrollChange);
    $('#autoscrollCheckbox').click();
  }

  function enterpressed() {
   Socket.send(document.getElementById('txbuff').value);
   document.getElementById('txbuff').value = '';
  }

  function reset(){
    Socket.send("CMD:RST");
  }
  function previousMode(){
    Socket.send("CMD:PREVIOUS_MODE");
  }
  function nextMode(){
    Socket.send("CMD:NEXT_MODE");
  }

  function onAutoscrollChange(){
    autoScroll = this.checked;
    //autoscrollCheckbox
    if(autoScroll) {
      var textArea = document.getElementById('rxConsole');
      textArea.scrollTo(0, textArea.scrollHeight);
    }
  }
</script>
</head>
<body onload='javascript:start();'>
  <div class="container">
    <ul class="nav nav-tabs" role="tablist">
      <li class="nav-item">
        <a class="nav-link active" id="Console-tab" data-toggle="tab" href="#Console" role="tab" aria-controls="Console" aria-selected="true">Console</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" id="Settings-tab" data-toggle="tab" href="#Settings" role="tab" aria-controls="Settings" aria-selected="false">Settings</a>
      </li>
    </ul>


    <div class="tab-content">
      <div class="tab-pane fade show active" id="Console" role="tabpanel" aria-labelledby="Console-tab">

        <div class="card">
          <div class="card-header">
            <div class="form-group row">
              <div class="col-sm-11">
                <input type='text' class="form-control" id='txbuff' onkeydown='if(event.keyCode == 13) enterpressed();'>
              </div>
              <input type='button' class="btn btn-danger" onclick='enterpressed();' value='Send' >
            </div>
          </div>
          <div class="card-body" style="padding: 5px;">
            <div class="form-group">
              <textarea class="form-control" id="rxConsole" readonly></textarea>
            </div> 
            <div class="form-check">
              <label class="form-check-label">
                <input type="checkbox" class="form-check-input" value="" id="autoscrollCheckbox">Auto scroll
              </label>
            </div>
          </div>
        </div>
        
        <div class="card">
          <div class="card-header">
            <h3>Actions</h3>
          </div>
          <div class="card-body">
            <div class="form-group row">
              <div class="col-sm-10">
                <input type='button' class="btn btn-primary" onclick='previousMode();' value='<- Previous Mode' >
                <input type='button' class="btn btn-success" onclick='nextMode();' value='Next Mode ->' >
              </div>
            </div>

          </div>
        </div>

      </div>


      <div class="tab-pane fade" id="Settings" role="tabpanel" aria-labelledby="Settings-tab">
        <div class="card">
          <div class="card-header">
            <h3>Actions</h3>
          </div>
          <div class="card-body">
            <div class="form-group row">
              <div class="col-sm-10">
                <input type='button' class="btn btn-danger" onclick='reset();' value='Reset ESP' >
              </div>
            </div>

          </div>
        </div>
      </div>
    </div>
  </div>


</body>
</html>

)=====";
