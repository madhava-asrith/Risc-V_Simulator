function autoResize()
{
    let maxheight = 500;
    const textArea = document.getElementById('input');

    textArea.style.height = 'auto';
    const scrollHeight = textArea.scrollHeight;

    if(scrollHeight > maxheight)
    {
        textArea.style.height = `${maxheight}px`;
        textArea.style.overflowY = 'auto';
    }
    else
    {
        textArea.style.height = `${scrollHeight}px`;
        textArea.style.overflowY = 'hidden';
    }

}

function resetValue()
{
    document.getElementById('file').value = '';
}

function loadFile() 
{
    var input = document.getElementById('file');
    var file = input.files[0];

    if(file !== '')
    {
        var reader = new FileReader();
        reader.onload = function() {
            document.getElementById('input').value = reader.result;
            autoResize();
        };
        reader.readAsText(file);   
    }

    set_regs();
}

function reset()
{
    document.getElementById('cache-output').value = '';
    set_regs();
    set_mem();
}

function set_regs()
{
    let input = document.getElementById('input').value;
    let reg_area = document.getElementById('reg_area');

    if(input === '')
    {
        reg_area.value = '';
        return;
    }
    reg_area.value = 'x0: 0x00';
    reg_area.value += '\nx1: 0x00';
    reg_area.value += '\nx2: 0x00';
    reg_area.value += '\nx3: 0x00';
    reg_area.value += '\nx4: 0x00';
    reg_area.value += '\nx5: 0x00';
    reg_area.value += '\nx6: 0x00';
    reg_area.value += '\nx7: 0x00';
    reg_area.value += '\nx8: 0x00';
    reg_area.value += '\nx9: 0x00';
    reg_area.value += '\nx10: 0x00';
    reg_area.value += '\nx11: 0x00';
    reg_area.value += '\nx12: 0x00';
    reg_area.value += '\nx13: 0x00';
    reg_area.value += '\nx14: 0x00';
    reg_area.value += '\nx15: 0x00';
    reg_area.value += '\nx16: 0x00';
    reg_area.value += '\nx17: 0x00';
    reg_area.value += '\nx18: 0x00';
    reg_area.value += '\nx19: 0x00';
    reg_area.value += '\nx20: 0x00';
    reg_area.value += '\nx21: 0x00';
    reg_area.value += '\nx22: 0x00';
    reg_area.value += '\nx23: 0x00';
    reg_area.value += '\nx24: 0x00';
    reg_area.value += '\nx25: 0x00';
    reg_area.value += '\nx26: 0x00';
    reg_area.value += '\nx27: 0x00';
    reg_area.value += '\nx28: 0x00';
    reg_area.value += '\nx29: 0x00';
    reg_area.value += '\nx30: 0x00';
    reg_area.value += '\nx31: 0x00';
}

function set_mem()
{
    let input = document.getElementById('input').value;
    let mem_area = document.getElementById('mem_area');

    if(input === '')
    {
        mem_area.value = '';
        return;
    }
    mem_area.value = '0x00: 0x00';
}

function run()
{
    let input = document.getElementById('input').value;

    if(input === '')
    {
        confirm('Please enter the assembly code');
        return;
    }

}

// for memory and register display -------------------------------------------
let button = document.getElementById('mem-btn');
button.addEventListener('click', function() {
    document.getElementById('memoryPopup').style.display = 'block';
});

button = document.getElementById('cancelButton');
button.addEventListener('click', function() {
    document.getElementById('memoryPopup').style.display = 'none';
});

button = document.getElementById('memory-btn');
button.addEventListener('click', function() {
    document.getElementById('memoryCont').style.display = 'block';
    document.getElementById('regsCont').style.display = 'none';
    document.getElementById('register-btn').style.display = 'block';
    document.getElementById('memory-btn').style.display = 'none';
});

button = document.getElementById('register-btn');
button.addEventListener('click', function() {
    document.getElementById('memoryCont').style.display = 'none';
    document.getElementById('regsCont').style.display = 'block';
    document.getElementById('register-btn').style.display = 'none';
    document.getElementById('memory-btn').style.display = 'block';
});

// for cache configuration ---------------------------------------------------
let config_button = document.getElementById('chng-config-btn');
config_button.addEventListener('click', function() {
    document.getElementById('cache-config').style.display = 'block';  
    document.getElementById('set-config').style.display = 'inline-block';  
    document.getElementById('cacheSize').removeAttribute('readonly');
    document.getElementById('blockSize').removeAttribute('readonly');
    document.getElementById('assoc').removeAttribute('readonly');
});

config_button = document.getElementById('config-btn');
config_button.addEventListener('click', function() {
    document.getElementById('cache-config').style.display = 'block';
    document.getElementById('set-config').style.display = 'none';
    document.getElementById('cacheSize').setAttribute('readonly', 'true');
    document.getElementById('blockSize').setAttribute('readonly', 'true');
    document.getElementById('assoc').setAttribute('readonly', 'true');
});

let cancel_button = document.getElementById('cancel-btn');
cancel_button.addEventListener('click', function() {
    document.getElementById('cache-config').style.display = 'none';
});

config_button = document.getElementById('set-config');
config_button.addEventListener('click', function() {
    let cacheSize = document.getElementById('cacheSize').value;
    let blockSize = document.getElementById('blockSize').value;
    let assoc = document.getElementById('assoc').value;

    if(cacheSize === '' || blockSize === '' || assoc === '')
    {
        confirm('Please enter all the fields');
        return;
    }
    document.getElementById('cache-enable').checked = true;
    document.getElementById('cache-config').style.display = 'none';
});

// for breakpoint -----------------------------------------------------------------------
let breakpoint_button = document.getElementById('set-breakpoint');
breakpoint_button.addEventListener('click', function() {
    document.getElementById('breakpointPopup').style.display = 'block';
});

breakpoint_button = document.getElementById('cancelBreakpoint');
breakpoint_button.addEventListener('click', function() {
    document.getElementById('breakpointPopup').style.display = 'none';
});

breakpoint_button = document.getElementById('show-breakpoint');
breakpoint_button.addEventListener('click', function() {
    document.getElementById('showBreakpointPopup').style.display = 'block';
});

breakpoint_button = document.getElementById('cancelShowBreakpoint');
breakpoint_button.addEventListener('click', function() {
    document.getElementById('showBreakpointPopup').style.display = 'none';
});

breakpoint_button = document.getElementById('clear-breakpoint');
breakpoint_button.addEventListener('click', function() {
    document.getElementById('showBreakpointArea').value = '';
});

breakpoint_button = document.getElementById('set-brkpt');
breakpoint_button.addEventListener('click', function() {
    let addr = document.getElementById('brkpt-addr').value;
    let showArea = document.getElementById('showBreakpointArea');
    showArea.value += "Line: " + addr + '\n';

    document.getElementById('breakpointPopup').style.display = 'none';
});

// for scroll to top button -------------------------------------------------------------
let mybutton = document.getElementById("myBtn");
window.onscroll = function() {scrollFunction()};
function scrollFunction() {
  if (document.body.scrollTop > 20 || document.documentElement.scrollTop > 20) {
    mybutton.style.display = "block";
  } else {
    mybutton.style.display = "none";
  }
}
function topFunction() {
  document.body.scrollTop = 0; // For Safari
  document.documentElement.scrollTop = 0; // For Chrome, Firefox, IE and Opera
}