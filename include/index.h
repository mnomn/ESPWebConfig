const char indexPage[] PROGMEM = R"INDEXRAW(
<!DOCTYPE html>
<head><style type=text/css>
body { margin:5%; font-family: Arial;}
form p label {display:block;float:left;width:100px;}
form p input {background-color:azure; margin-bottom: 2%;}
</style></head>
<body><h1>Configure device</h1>
<form id=form action=/parameters method=PUT><h3 id="htxt"></h3><p id="par"></p><input type=submit value=Save></form>
<script>
let d = document
d.addEventListener('DOMContentLoaded', (event) => {getParameters();});
function popH(t) {
if (t) d.getElementById("htxt").innerText = t;
}
function popF(par, id) {
if (!par || !par.n) return
let ll=d.getElementById("par")
let lab=d.createElement("label")
// Label and and type
let lt=par.n.split('|')
let l=lt[0]
lab.innerText=l
let inp=d.createElement("input")
inp.id = String(id)
inp.type=lt.length>1?lt[1]:"text"
inp.value=par.v
if (l.endsWith('*')) inp.required="required";
ll.appendChild(lab)
ll.appendChild(inp)
ll.appendChild(d.createElement("br"))
}
function getParameters() {
fetch("/parameters")
.then((data)=> data.json())
.then(res=>{
popH(res.helpText)
if (res.properties) {let id=1;for(const p in obj.properties) {popF(p,id)}}
})
.catch((err) => {
popH("Error, cannot fetch parameters.");
})
}
</script></body>
)INDEXRAW";
