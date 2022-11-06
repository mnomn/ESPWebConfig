const char indexPage[] PROGMEM = R"INDEXRAW(
<!DOCTYPE html>
<head><style type=text/css>
body { margin:5%; font-family: Arial;}
form p label {display:block;float:left;width:100px;}
form p input {background-color:azure; margin-bottom: 2%;}
</style></head>
<body><h1>Device configuration</h1>
<form id=form action=/ method=POST><h3>Configure Wifi</h3><p id="par"></p>
<input type=submit value=Save></form>
<h6 id="cid"></h6>
<script>
let d = document

d.addEventListener('DOMContentLoaded', (event) => {getParameters(true);});
function popC(c) {
if (!c) return;
let cc=d.getElementById("cid")
if (!cc) return
cc.innerText="Chip id: " + c
}
function popH(t) {
if (!t) return;
let ll=d.getElementById("par")
if (!ll) return
let h3=d.createElement("h3")
h3.innerText=t
ll.appendChild(h3)
}
function popF(pl, ix) {
const par=pl[ix] // Array index [0..3]
const id = ix+1 // Paramid [1..4]
let ll=d.getElementById("par")
let lab=d.createElement("label")
// Label and and type
let lt=par.n.split('|')
let l=lt[0]
lab.innerText=l
let inp=d.createElement("input")
inp.name = ''+id
inp.type=lt.length>1?lt[1]:"text"
inp.value=par.v
if (l.endsWith('*')) inp.required="required";
ll.appendChild(lab)
ll.appendChild(inp)
ll.appendChild(d.createElement("br"))
}
function getParameters(getValues) {
let path = "/parameters"
if (getValues) path += "?getValues=1"
fetch(path)
.then((data)=> data.json())
.then(res=>{
if (res.parameters) {
for(let ix=0;ix<res.parameters.length;ix++){
if (ix == 2) popH(res.helpText)
popF(res.parameters, ix)}
}
popC(res.chipId)
})
.catch((err) => {
// if "getValues was set, retry without get values"
if (getValues) getParameters(false)
else popH("Cannot fetch parameters." + err);
})
}
</script></body>
)INDEXRAW";
