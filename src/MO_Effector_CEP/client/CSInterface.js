/* Adobe CEP bridge — minimal compatible CSInterface facade. */
function CSInterface(){this.evalScript=function(script,callback){window.__adobe_cep__.evalScript(script,callback);};}
