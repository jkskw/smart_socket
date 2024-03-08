// Toggling the socket on or off
function toggle_relay(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?relay_state=1", true);
  } else {
    xhr.open("GET", "/update?relay_state=0", true);
  }
  xhr.send();
}

// Toggling the night light mode on or off
function toggle_nightlight(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?nightlight_state=1", true);
  } else {
    xhr.open("GET", "/update?nightlight_state=0", true);
  }
  xhr.send();

  setTimeout(function () {
    document.location.reload();
  }, 1000);
}

// Toggling the temperature mode on or off
function toggle_temp(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?temp_state=1", true);
  } else {
    xhr.open("GET", "/update?temp_state=0", true);
  }
  xhr.send();

    setTimeout(function () {
      document.location.reload();
    }, 1000); 
}

// Toggling the schedule mode on or off
function toggle_sched(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?sched_state=1", true);
  } else {
    xhr.open("GET", "/update?sched_state=0", true);
  }
  xhr.send();

    setTimeout(function () {
      document.location.reload();
    }, 1000); 
}

// Toggling the security mode on or off
function toggle_safety(element) {
  var xhr = new XMLHttpRequest();
  if (element.checked) {
    xhr.open("GET", "/update?safety_state=1", true);
  } else {
    xhr.open("GET", "/update?safety_state=0", true);
  }
  xhr.send();

    setTimeout(function () {
      document.location.reload();
    }, 1000); 
}

// Resetting the power consumption state
function btn_reset(element) {
  var xhr = new XMLHttpRequest();
  setTimeout(function () {
    document.location.reload();
  }, 500);
  xhr.open("GET", "/reset");
  xhr.send();
}

// Resetting the security settings
function safety_reset(element) {
  var xhr = new XMLHttpRequest();
  setTimeout(function () {
    document.location.reload();
  }, 500);
  xhr.open("GET", "/safety_reset");
  xhr.send();
}

// Refreshing the page
function refresh() {
  setTimeout(function () {
    document.location.reload();
  }, 500);
}

// Retrieving and updating the socket state at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if (this.responseText == 0) {
        inputChecked = true;
        outputStateM = "on";
      } else {
        inputChecked = false;
        outputStateM = "off";
      }
      document.getElementById("relay_output").checked = inputChecked;
      document.getElementById("relay_outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/relay_state", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the night light mode state at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if (this.responseText == 1) {
        inputChecked = true;
        outputStateM = "on";
      } else {
        inputChecked = false;
        outputStateM = "off";
      }
      document.getElementById("nightlight_output").checked = inputChecked;
      document.getElementById("nightlight_outputState").innerHTML =
        outputStateM;
    }
  };
  xhttp.open("GET", "/nightlight_state", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the temperature mode state at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if (this.responseText == 1) {
        inputChecked = true;
        outputStateM = "on";
      } else {
        inputChecked = false;
        outputStateM = "off";
      }
      document.getElementById("temp_output").checked = inputChecked;
      document.getElementById("temp_outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/temp_state", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the schedule mode state at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if (this.responseText == 1) {
        inputChecked = true;
        outputStateM = "on";
      } else {
        inputChecked = false;
        outputStateM = "off";
      }
      document.getElementById("sched_output").checked = inputChecked;
      document.getElementById("sched_outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/sched_state", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the security mode state at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if (this.responseText == 1) {
        inputChecked = true;
        outputStateM = "on";
      } else {
        inputChecked = false;
        outputStateM = "off";
      }
      document.getElementById("safety_output").checked = inputChecked;
      document.getElementById("safety_outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/safety_state", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the temperature reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

// Retrieving and updating the humidity reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);

// Retrieving and updating the current reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("current").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/current", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the voltage reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("voltage").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/voltage", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the power reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("power").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/power", true);
  xhttp.send();
}, 1000);

// Retrieving and updating the total energy consumption reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("kWh").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/kWh", true);
  xhttp.send();
}, 5000);

// Retrieving and updating the total cost of energy consumption reading at specified intervals
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("total_cost").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/total_cost", true);
  xhttp.send();
}, 5000);

// Retrieving and updating the security alerts
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("alarm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/alarm", true);
  xhttp.send();
}, 5000);

// Function handling the logout button
function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function () {
    window.location.href = "/logout_html.html";
  }, 1000);
}

function showPassword1() {
  var x = document.getElementById("pw");
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
}

function showPassword2() {
  var x = document.getElementById("user_pw");
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
}

/*
function configDone() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/config", true);
  xhr.send();
  setTimeout(function () {
    window.location.href = "/config.html";
  }, 1000);
}
*/