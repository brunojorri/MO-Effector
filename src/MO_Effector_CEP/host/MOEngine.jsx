/* MO Effector host engine. The CEP panel calls MOEffector.create(). */
$.global.MOEffector = (function(){
  function slider(layer,name,value){var effect=layer.property("ADBE Effect Parade").addProperty("ADBE Slider Control");effect.name=name;effect.property(1).setValue(value);}
  function checkbox(layer,name,value){var effect=layer.property("ADBE Effect Parade").addProperty("ADBE Checkbox Control");effect.name=name;effect.property(1).setValue(value?1:0);}
  function layerControl(layer,name,target){var effect=layer.property("ADBE Effect Parade").addProperty("ADBE Layer Control");effect.name=name;if(target)effect.property(1).setValue(target.index);}
  function controllerPrefix(){return "var c=effect(\"MO Controller\")(\"Layer\");";}
  function ref(group,name,grouped){return grouped?'c.effect("MO Cloner Controls")("'+name+'")':'c.effect("'+name+'")("'+(name.indexOf("Reverse")>=0||name.indexOf("Clamp")>=0?'Checkbox':'Slider')+'")';}
  function effectorPrefix(grouped,point){var sample=point||"toComp(anchorPoint)";return controllerPrefix()+"var ep="+sample+";var shape=Math.round("+ref("Advanced Falloff","Effector Shape",grouped)+");var ir="+ref("Effector","Inner Radius",grouped)+";var or=Math.max(ir+.01,"+ref("Effector","Radius",grouped)+");var ft=Math.round("+ref("Advanced Falloff","Falloff Type",grouped)+");var pw=Math.max(.1,"+ref("Advanced Falloff","Falloff Power",grouped)+");function moInf(e){if(!e)return 0;var ec=e.toComp(e.anchorPoint);var dx=ep[0]-ec[0];var dy=ep[1]-ec[1];var ea=e.transform.rotation*Math.PI/180;var exs=Math.max(.0001,Math.abs(e.transform.scale[0])/100);var eys=Math.max(.0001,Math.abs(e.transform.scale[1])/100);var lx=(dx*Math.cos(ea)+dy*Math.sin(ea))/exs;var ly=(-dx*Math.sin(ea)+dy*Math.cos(ea))/eys;var d=shape==1?Math.max(Math.abs(lx),Math.abs(ly)):shape==2?Math.abs(lx):shape==3?Math.abs(ly):Math.sqrt(lx*lx+ly*ly);var u=clamp((d-ir)/(or-ir),0,1);var f;if(ft==1)f=1-u*u*(3-2*u);else if(ft==2)f=Math.pow(1-u,pw);else if(ft==3)f=1-Math.pow(u,pw);else if(ft==4){var ge=Math.exp(-pw);f=(Math.exp(-pw*u*u)-ge)/Math.max(.0001,1-ge);}else if(ft==5)f=d<=or?1:0;else f=1-u;if("+ref("Advanced Falloff","Falloff Invert",grouped)+"==1)f=d<=or?1-f:0;var w=e.transform.opacity/100;return clamp(f,0,1)*"+ref("Effector","Strength",grouped)+"/100*w;}var ecnt=Math.max(1,Math.min(4,Math.round(c.effect(\"MO Effector Count\")(\"Slider\"))));var e1=c.effect(\"MO Effector\")(\"Layer\");var e2=ecnt>=2?c.effect(\"MO Effector 2\")(\"Layer\"):null;var e3=ecnt>=3?c.effect(\"MO Effector 3\")(\"Layer\"):null;var e4=ecnt>=4?c.effect(\"MO Effector 4\")(\"Layer\"):null;var i1=moInf(e1),i2=moInf(e2),i3=moInf(e3),i4=moInf(e4);var cm=Math.round(c.effect(\"MO Combine Mode\")(\"Slider\"));var i;if(cm==1)i=clamp(i1+i2+i3+i4,0,1);else if(cm==2){var mul=1,active=0;if(ecnt>=1){mul*=i1;active++;}if(ecnt>=2){mul*=i2;active++;}if(ecnt>=3){mul*=i3;active++;}if(ecnt>=4){mul*=i4;active++;}i=active?mul:0;}else if(cm==3)i=clamp(i1-i2-i3-i4,0,1);else i=Math.max(i1,i2,i3,i4);";}
  function stepPrefix(grouped){return controllerPrefix()+"var n=effect(\"MO Index\")(\"Slider\")-1;var total=Math.max(1,Math.round("+ref("Formation","Clone Count",grouped)+"));var t=(n+"+ref("Step Stagger","Step Offset",grouped)+")/Math.max(1,total-1);if("+ref("Step Stagger","Step Reverse",grouped)+"==1)t=1-t;var f=clamp("+ref("Step Stagger","Step Falloff",grouped)+"/100,.01,1);t=(t-(1-f)/2)/f;if("+ref("Step Stagger","Step Clamp",grouped)+"==1)t=clamp(t,0,1);seedRandom(n+"+ref("Step Stagger","Step Random Seed",grouped)+",true);t+=random(-1,1)*"+ref("Step Stagger","Step Randomise",grouped)+"/100;";}
  function noisePrefix(grouped,offset){return grouped?"seedRandom(n+"+ref("Noise","Noise Seed",true)+"+"+offset+",true);":"";}
  function noiseValue(offset){return "noise(n*12.9898+"+ref("Noise","Noise Seed",true)+"+"+offset+"+time*"+ref("Noise","Noise Speed",true)+")";}
  function globeLayout(grouped){
    return "var gd=Math.round("+ref("Globe Pro","Globe Distribution",grouped)+");var ga=Math.PI*(3-Math.sqrt(5));var gx,gy,gz;if(gd==1){var rings=Math.max(2,Math.round("+ref("Globe Pro","Globe Rings",grouped)+"));var lonCount=Math.max(1,Math.ceil(total/rings));var ring=Math.min(rings-1,Math.floor(n/lonCount));var lat=-Math.PI/2+Math.PI*(ring+.5)/rings;var lon=(n%lonCount)/lonCount*Math.PI*2;gy=Math.sin(lat);var ringRadius=Math.cos(lat);gx=Math.cos(lon)*ringRadius;gz=Math.sin(lon)*ringRadius;}else if(gd==2){seedRandom(n+"+ref("Globe Pro","Globe Seed",grouped)+",true);gy=random(-1,1);var randomAngle=random(0,Math.PI*2);var randomRadius=Math.sqrt(Math.max(0,1-gy*gy));gx=Math.cos(randomAngle)*randomRadius;gz=Math.sin(randomAngle)*randomRadius;}else{gy=1-2*(n+.5)/total;var gr=Math.sqrt(Math.max(0,1-gy*gy));gx=Math.cos(n*ga)*gr;gz=Math.sin(n*ga)*gr;}var twist=gy*"+ref("Globe Pro","Globe Twist",grouped)+"*Math.PI/180;var twistedX=gx*Math.cos(twist)-gz*Math.sin(twist);var twistedZ=gx*Math.sin(twist)+gz*Math.cos(twist);gx=twistedX;gz=twistedZ;var ax=("+ref("Globe","Globe Rotation X",grouped)+"+time*"+ref("Globe","Globe Spin X",grouped)+")*Math.PI/180;var ay=("+ref("Globe","Globe Rotation Y",grouped)+"+time*"+ref("Globe","Globe Spin Y",grouped)+")*Math.PI/180;var az=("+ref("Globe","Globe Rotation Z",grouped)+"+time*"+ref("Globe","Globe Spin Z",grouped)+")*Math.PI/180;var y1=gy*Math.cos(ax)-gz*Math.sin(ax);var z1=gy*Math.sin(ax)+gz*Math.cos(ax);var x2=gx*Math.cos(ay)+z1*Math.sin(ay);var z2=-gx*Math.sin(ay)+z1*Math.cos(ay);var x3=x2*Math.cos(az)-y1*Math.sin(az);var y3=x2*Math.sin(az)+y1*Math.cos(az);var globeDepth=z2;var globePerspective=Math.max(.05,1+globeDepth*"+ref("Globe Pro","Globe Perspective",grouped)+"/100);var globeRadius="+ref("Globe","Globe Radius",grouped)+";var globeWidth="+ref("Globe Pro","Globe Width",grouped)+"/100;var globeHeight="+ref("Globe Pro","Globe Height",grouped)+"/100;p=o+[x3*globeRadius*globeWidth*globePerspective,y3*globeRadius*globeHeight*globePerspective];var globeSurfaceAngle=Math.atan2(y3*globeHeight,x3*globeWidth)*180/Math.PI+90;";
  }
  function layoutPrefix(mode,grouped){
    var origin=grouped?"var o=["+ref("Formation","Cloner Position X",true)+","+ref("Formation","Cloner Position Y",true)+"];":"var src=c.effect(\"MO Source\")(\"Layer\");var o=src.toComp(src.anchorPoint);";
    var layout=controllerPrefix()+origin+"var n=effect(\"MO Index\")(\"Slider\")-1;var cols=Math.max(1,Math.round("+ref("Grid","Grid Columns",grouped)+"));var total=Math.max(1,Math.round("+ref("Formation","Clone Count",grouped)+"));var sx="+ref("Grid","Spacing X",grouped)+";var sy="+ref("Grid","Spacing Y",grouped)+";var p;";
    if(mode==="globe")layout+=globeLayout(grouped);
    else if(mode==="path")layout+="var pathLayer=c.effect(\"MO Path\")(\"Layer\");var path=pathLayer.content(\"MO Path\").content(\"Path 1\").path;var pathStart="+ref("Path Cloner","Path Start",grouped)+"/100;var pathEnd="+ref("Path Cloner","Path End",grouped)+"/100;var pathU=linear(total<=1 ? 0.5 : n/(total-1),pathStart,pathEnd)+"+ref("Path Cloner","Path Offset",grouped)+"/100;if("+ref("Path Cloner","Path Reverse",grouped)+"==1)pathU=1-pathU;if("+ref("Path Cloner","Path Loop",grouped)+"==1)pathU=((pathU%1)+1)%1;else pathU=clamp(pathU,0,1);p=pathLayer.toComp(path.pointOnPath(pathU));var pathA=Math.max(0,pathU-.001),pathB=Math.min(1,pathU+.001);var pathPA=pathLayer.toComp(path.pointOnPath(pathA)),pathPB=pathLayer.toComp(path.pointOnPath(pathB));var pathAngle=Math.atan2(pathPB[1]-pathPA[1],pathPB[0]-pathPA[0])*180/Math.PI;";
    else if(mode==="linear")layout+="p=o+[(n-(total-1)/2)*sx,0];";
    else if(mode==="radial")layout+="var a=(n/total)*Math.PI*2-Math.PI/2;p=o+[Math.cos(a)*sx,Math.sin(a)*sx];";
    else if(mode==="scatter")layout+=noisePrefix(grouped,0)+"var rows=Math.max(1,Math.ceil(total/cols));p=o+[random(-sx*cols/2,sx*cols/2),random(-sy*rows/2,sy*rows/2)];";
    else layout+="var row=Math.floor(n/cols);var col=n%cols;p=o+[(col-(cols-1)/2)*sx,(row-(Math.ceil(total/cols)-1)/2)*sy];";
    return layout;
  }
  function positionExpression(mode,grouped){
    var layout=layoutPrefix(mode,grouped);
    var noise=grouped?"var nx="+noiseValue(11)+"*"+ref("Noise","Noise Position X",true)+";var ny="+noiseValue(37)+"*"+ref("Noise","Noise Position Y",true)+";":"var nx=0;var ny=0;";
    return layout+effectorPrefix(grouped,"p")+stepPrefix(grouped)+noise+"p+["+ref("Effector","Position X",grouped)+"*i+"+ref("Step Stagger","Step Position X",grouped)+"*t+nx,"+ref("Effector","Position Y",grouped)+"*i+"+ref("Step Stagger","Step Position Y",grouped)+"*t+ny];";
  }
  function expression(type,mode,grouped){
    if(type==="p")return positionExpression(mode,grouped);
    var prefix=layoutPrefix(mode,grouped)+effectorPrefix(grouped,"p")+stepPrefix(grouped);
    if(type==="s"){var ns=grouped?"var ns="+noiseValue(101)+"*"+ref("Noise","Noise Scale",true)+"/100;":"var ns=0;";var gs=mode==="globe"?"var gs=linear(globeDepth,-1,1,"+ref("Globe","Globe Back Scale",grouped)+","+ref("Globe","Globe Front Scale",grouped)+")/100*globePerspective;":"var gs=1;";return prefix+ns+gs+"value*("+ref("Formation","Clone Size",grouped)+"/100)*(1+"+ref("Effector","Scale Amount",grouped)+"/100*i)*(1+"+ref("Step Stagger","Step Scale",grouped)+"/100*t)*(1+ns)*gs;";}
    if(type==="r"){var nr=grouped?noiseValue(202)+"*"+ref("Noise","Noise Rotation",true):"0";var pr=mode==="path"?"var layoutRotation="+ref("Path Cloner","Orient to Path",grouped)+"==1?pathAngle+"+ref("Path Cloner","Path Rotation Offset",grouped)+":0;":mode==="globe"?"var layoutRotation="+ref("Globe Pro","Globe Surface Orient",grouped)+"==1?globeSurfaceAngle+"+ref("Globe Pro","Globe Orient Offset",grouped)+":0;":"var layoutRotation=0;";return prefix+pr+"value+layoutRotation+"+ref("Effector","Rotation Amount",grouped)+"*i+"+ref("Step Stagger","Step Rotation",grouped)+"*t+"+nr+";";}
    var go=mode==="globe"?"var go=linear(globeDepth,-1,1,"+ref("Globe","Globe Back Opacity",grouped)+",100)/100;":"var go=1;";return prefix+go+"var eff=value+("+ref("Effector","Target Opacity",grouped)+"-value)*i;var out=eff+("+ref("Step Stagger","Step Opacity",grouped)+"-eff)*t;out*go;";
  }
  function colorExpression(mode){
    var prefix=layoutPrefix(mode,true)+effectorPrefix(true,"p");
    var depth=mode==="globe"?"var depthBase=linear(globeDepth,-1,1,back,front);base=base+(depthBase-base)*depthOn;":"";
    return prefix+"var base="+ref("Color Effector","Base Color",true)+";var target="+ref("Color Effector","Effector Color",true)+";var back="+ref("Color Effector","Globe Back Color",true)+";var front="+ref("Color Effector","Globe Front Color",true)+";var depthOn="+ref("Color Effector","Globe Depth Color",true)+";"+depth+"var amount=clamp("+ref("Color Effector","Color Amount",true)+"/100,0,1);var mixed=base+(target-base)*clamp(i*amount,0,1);seedRandom(n+"+ref("Color Effector","Color Seed",true)+",true);var randomColor=[random(),random(),random(),1];var randomAmount=clamp("+ref("Color Effector","Random Color",true)+"/100,0,1);mixed+(randomColor-mixed)*randomAmount;";
  }
  function colorOpacityExpression(){return controllerPrefix()+ref("Color Effector","Enable Color",true)+"==1?100:0;";}
  function controllerEffect(layer,name){return layer.property("ADBE Effect Parade").property(name);}
  function setSlider(layer,name,value){var effect=controllerEffect(layer,name);if(effect)effect.property(1).setValue(value);else slider(layer,name,value);}
  function setLayerControl(layer,name,target){var effect=controllerEffect(layer,name);if(effect)effect.property(1).setValue(target.index);else layerControl(layer,name,target);}
  function ensureMultiControls(controller){if(!controllerEffect(controller,"MO Combine Mode"))slider(controller,"MO Combine Mode",0);for(var slot=2;slot<=4;slot++){var name="MO Effector "+slot;if(!controllerEffect(controller,name))layerControl(controller,name,null);}var detected=1;for(var s=2;s<=4;s++){var control=controllerEffect(controller,"MO Effector "+s);if(control&&Math.round(control.property(1).value)>0)detected=s;}var count=controllerEffect(controller,"MO Effector Count");if(!count)slider(controller,"MO Effector Count",detected);else if(count.property(1).value<detected)count.property(1).setValue(detected);}
  function sourceControlName(slot){return slot===1?"MO Source":"MO Source "+slot;}
  function ensureSourceControls(controller,sources){var count=sources?sources.length:1;setSlider(controller,"MO Source Count",count);if(sources)setLayerControl(controller,"MO Source",sources[0]);for(var slot=2;slot<=count;slot++)setLayerControl(controller,sourceControlName(slot),sources[slot-1]);}
  function sourceForIndex(comp,controller,pc,index){var countControl=controllerEffect(controller,"MO Source Count"),count=Math.max(1,Math.min(8,Math.round(countControl?countControl.property(1).value:1)));var modeParam=pc?pseudoParam(pc,"Multi Source","Source Mode"):null,offsetParam=pc?pseudoParam(pc,"Multi Source","Source Offset"):null,seedParam=pc?pseudoParam(pc,"Multi Source","Source Seed"):null;var mode=Math.round(modeParam?modeParam.value:0),offset=Math.round(offsetParam?offsetParam.value:0),seed=Math.round(seedParam?seedParam.value:1),slot;if(mode===1){var raw=Math.abs(Math.sin((index+offset+seed)*12.9898)*43758.5453),fraction=raw-Math.floor(raw);slot=Math.floor(fraction*count)+1;}else slot=((index-1+offset)%count+count)%count+1;var control=controllerEffect(controller,sourceControlName(slot)),layer=control&&Math.round(control.property(1).value)>0?comp.layer(Math.round(control.property(1).value)):null;if(!layer)throw new Error("MO Source "+slot+" nao foi encontrado.");return {layer:layer,slot:slot};}
  function effectorControlName(slot){return slot===1?"MO Effector":"MO Effector "+slot;}
  function linkedEffector(comp,controller,slot){var control=controllerEffect(controller,effectorControlName(slot));if(!control)return null;var index=Math.round(control.property(1).value);return index>0?comp.layer(index):null;}
  function pseudo(controller){return controllerEffect(controller,"MO Cloner Controls");}
  function findParam(group,name){var direct=group.property(name);if(direct&&(!direct.numProperties||direct.numProperties===0))return direct;for(var i=1;i<=group.numProperties;i++){var item=group.property(i);if(item.name===name&&(!item.numProperties||item.numProperties===0))return item;if(item.numProperties){var found=findParam(item,name);if(found)return found;}}return null;}
  function pseudoParam(control,group,name){return findParam(control,name);}
  function setPseudo(control,group,name,value){pseudoParam(control,group,name).setValue(value);}
  function guidePath(root,name,pathMatch,sizeMatch,color,opacityExpression,sizeExpression){
    var group=root.addProperty("ADBE Vector Group");group.name=name;
    var vectors=group.property("ADBE Vectors Group");
    var path=vectors.addProperty(pathMatch);path.name=name;
    path.property(sizeMatch).expression=sizeExpression;
    var stroke=vectors.addProperty("ADBE Vector Graphic - Stroke");stroke.name=name+" Stroke";
    stroke.property("ADBE Vector Stroke Color").setValue(color);
    stroke.property("ADBE Vector Stroke Opacity").expression=opacityExpression;
    stroke.property("ADBE Vector Stroke Width").setValue(2);
  }
  function createVisualEffector(comp,controller,position,name){
    var effector=comp.layers.addShape();effector.name=name||"MO Effector";effector.label=10;effector.guideLayer=true;
    effector.property("ADBE Transform Group").property("ADBE Position").setValue(position);
    layerControl(effector,"MO Controller",controller);
    var root=effector.property("ADBE Root Vectors Group");
    var prefix='var c=effect("MO Controller")("Layer");';
    var shapeRef='Math.round(c.effect("MO Cloner Controls")("Effector Shape"))';
    var outer=prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Radius"));[r*2,r*2];';
    var inner=prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Inner Radius"));[r*2,r*2];';
    guidePath(root,"Circle Outer","ADBE Vector Shape - Ellipse","ADBE Vector Ellipse Size",[0,0.75,1],prefix+shapeRef+'==0?100:0;',outer);
    guidePath(root,"Circle Inner","ADBE Vector Shape - Ellipse","ADBE Vector Ellipse Size",[0.2,1,0.65],prefix+shapeRef+'==0?55:0;',inner);
    guidePath(root,"Box Outer","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0,0.75,1],prefix+shapeRef+'==1?100:0;',outer);
    guidePath(root,"Box Inner","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0.2,1,0.65],prefix+shapeRef+'==1?55:0;',inner);
    guidePath(root,"Linear X Outer","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0,0.75,1],prefix+shapeRef+'==2?100:0;',prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Radius"));[r*2,thisComp.height*2];');
    guidePath(root,"Linear X Inner","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0.2,1,0.65],prefix+shapeRef+'==2?55:0;',prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Inner Radius"));[r*2,thisComp.height*2];');
    guidePath(root,"Linear Y Outer","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0,0.75,1],prefix+shapeRef+'==3?100:0;',prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Radius"));[thisComp.width*2,r*2];');
    guidePath(root,"Linear Y Inner","ADBE Vector Shape - Rect","ADBE Vector Rect Size",[0.2,1,0.65],prefix+shapeRef+'==3?55:0;',prefix+'var r=Math.max(0,c.effect("MO Cloner Controls")("Inner Radius"));[thisComp.width*2,r*2];');
    return effector;
  }
  function createPathGuide(comp,controller){
    var layer=comp.layers.addShape();layer.name="MO Cloner Path";layer.label=14;layer.guideLayer=true;
    layer.property("ADBE Transform Group").property("ADBE Position").setValue([comp.width/2,comp.height/2]);layerControl(layer,"MO Controller",controller);
    var root=layer.property("ADBE Root Vectors Group"),group=root.addProperty("ADBE Vector Group");group.name="MO Path";
    var vectors=group.property("ADBE Vectors Group"),pathGroup=vectors.addProperty("ADBE Vector Shape - Group");pathGroup.name="Path 1";
    var shape=new Shape();shape.vertices=[[-420,0],[-140,-150],[140,150],[420,0]];shape.inTangents=[[0,0],[-95,0],[-95,0],[-95,0]];shape.outTangents=[[95,0],[95,0],[95,0],[0,0]];shape.closed=false;
    pathGroup.property("ADBE Vector Shape").setValue(shape);
    var stroke=vectors.addProperty("ADBE Vector Graphic - Stroke");stroke.name="Path Guide";stroke.property("ADBE Vector Stroke Color").setValue([0.1,0.8,1]);stroke.property("ADBE Vector Stroke Width").setValue(3);
    return layer;
  }
  function createSourceShape(kind,size,sides){
    var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";
    size=Math.max(10,Math.min(2000,Number(size)||120));sides=Math.max(3,Math.min(20,Math.round(Number(sides)||6)));
    app.beginUndoGroup("MO Effector - Create Source Shape");var layer=null;
    try{
      for(var i=1;i<=comp.numLayers;i++)comp.layer(i).selected=false;
      layer=comp.layers.addShape();layer.label=11;layer.guideLayer=false;
      layer.property("ADBE Transform Group").property("ADBE Position").setValue([comp.width/2,comp.height/2]);
      var root=layer.property("ADBE Root Vectors Group"),group=root.addProperty("ADBE Vector Group"),vectors=group.property("ADBE Vectors Group"),path;
      if(kind==="square"){
        layer.name="MO Source - Square";group.name="Square";path=vectors.addProperty("ADBE Vector Shape - Rect");path.property("ADBE Vector Rect Size").setValue([size,size]);
      }else if(kind==="polygon"){
        layer.name="MO Source - Polygon "+sides;group.name="Polygon "+sides;path=vectors.addProperty("ADBE Vector Shape - Star");path.property("ADBE Vector Star Type").setValue(2);path.property("ADBE Vector Star Points").setValue(sides);path.property("ADBE Vector Star Outer Radius").setValue(size/2);
      }else{
        layer.name="MO Source - Circle";group.name="Circle";path=vectors.addProperty("ADBE Vector Shape - Ellipse");path.property("ADBE Vector Ellipse Size").setValue([size,size]);
      }
      var fill=vectors.addProperty("ADBE Vector Graphic - Fill");fill.property("ADBE Vector Fill Color").setValue([0.94,0.95,0.98]);
      var stroke=vectors.addProperty("ADBE Vector Graphic - Stroke");stroke.property("ADBE Vector Stroke Color").setValue([0,0.68,0.9]);stroke.property("ADBE Vector Stroke Width").setValue(3);
      layer.selected=true;app.endUndoGroup();return "OK:"+layer.name;
    }catch(err){try{if(layer)layer.remove();}catch(cleanup){}app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function clonesFor(comp,controller){var clones=[];for(var i=1;i<=comp.numLayers;i++){var layer=comp.layer(i),control=controllerEffect(layer,"MO Controller"),index=controllerEffect(layer,"MO Index");if(control&&index&&Math.round(control.property(1).value)===controller.index)clones.push(layer);}clones.sort(function(a,b){return controllerEffect(a,"MO Index").property(1).value-controllerEffect(b,"MO Index").property(1).value;});return clones;}
  function controllerMode(controller,clones){var pc=pseudo(controller),stored=pc?pseudoParam(pc,"Formation","Cloner Mode"):controllerEffect(controller,"Cloner Mode");if(stored){var v=Math.round(pc?stored.value:stored.property(1).value);return v===1?"linear":v===2?"radial":v===3?"scatter":v===4?"globe":v===5?"path":"grid";}if(clones.length){var x=clones[0].property("ADBE Transform Group").property("ADBE Position").expression;if(x.indexOf("pointOnPath")!==-1)return "path";if(x.indexOf("globeDepth")!==-1)return "globe";if(x.indexOf("Math.cos(a)")!==-1)return "radial";if(x.indexOf("(n-(total-1)/2)")!==-1)return "linear";}return "grid";}
  function fillParam(effect,name,fallback){var property=effect.property(name);return property||effect.property(fallback);}
  function applyColorExpression(clone,mode){
    var parade=clone.property("ADBE Effect Parade"),fill=parade.property("MO Color");
    if(!fill){fill=parade.addProperty("ADBE Fill");fill.name="MO Color";}
    var color=fillParam(fill,"Color",3),opacity=fillParam(fill,"Opacity",7);
    if(!color||!opacity)throw new Error("Os parametros Color/Opacity do efeito Fill nao foram encontrados.");
    color.expression=colorExpression(mode);opacity.expression=colorOpacityExpression();
  }
  function applyExpressions(clone,controller,index,mode,grouped,sourceSlot){clone.name="MO Clone "+index;setLayerControl(clone,"MO Controller",controller);setSlider(clone,"MO Index",index);if(sourceSlot!==undefined)setSlider(clone,"MO Source Slot",sourceSlot);clone.property("ADBE Transform Group").property("ADBE Position").expression=expression("p",mode,grouped);clone.property("ADBE Transform Group").property("ADBE Scale").expression=expression("s",mode,grouped);clone.property("ADBE Transform Group").property("ADBE Rotate Z").expression=expression("r",mode,grouped);clone.property("ADBE Transform Group").property("ADBE Opacity").expression=expression("o",mode,grouped);if(grouped)applyColorExpression(clone,mode);}
  function syncCloneCount(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Controller.";var controller=comp.selectedLayers[0],pc=pseudo(controller),countEffect=pc?pseudoParam(pc,"Formation","Clone Count"):controllerEffect(controller,"Clone Count"),sourceEffect=controllerEffect(controller,"MO Source");if(!countEffect||!sourceEffect)return "Selecione um MO Controller valido.";ensureMultiControls(controller);if(!controllerEffect(controller,"MO Source Count"))ensureSourceControls(controller,null);var desired=Math.round(pc?countEffect.value:countEffect.property(1).value);if(desired<1||desired>400)return "Clone Count deve estar entre 1 e 400.";app.beginUndoGroup("MO Effector - Sync Clone Count");try{var clones=clonesFor(comp,controller),mode=controllerMode(controller,clones),source=comp.layer(Math.round(sourceEffect.property(1).value));if(!source)throw new Error("MO Source nao foi encontrado.");if(pc&&clones.length&&clones[0].property("ADBE Transform Group").property("ADBE Position").expression.indexOf("Cloner Position X")===-1){var oldPosition=source.property("ADBE Transform Group").property("ADBE Position").value;setPseudo(pc,"Formation","Cloner Position X",oldPosition[0]);setPseudo(pc,"Formation","Cloner Position Y",oldPosition[1]);}while(clones.length>desired){clones[clones.length-1].remove();clones.pop();}for(var i=0;i<desired;i++){var info=sourceForIndex(comp,controller,pc,i+1),clone=clones[i],slotControl=clone?controllerEffect(clone,"MO Source Slot"):null,currentSlot=slotControl?Math.round(slotControl.property(1).value):1;if(!clone||currentSlot!==info.slot){if(clone)clone.remove();clone=info.layer.duplicate();clone.enabled=true;clones[i]=clone;}applyExpressions(clone,controller,i+1,mode,!!pc,info.slot);}app.endUndoGroup();return "OK:"+desired;}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function centerCloner(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Controller.";var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Controller da versao 0.8 ou superior.";app.beginUndoGroup("MO Effector - Center Cloner");try{setPseudo(pc,"Formation","Cloner Position X",comp.width/2);setPseudo(pc,"Formation","Cloner Position Y",comp.height/2);app.endUndoGroup();return "OK";}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function setEffectorShape(shape){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var pc=pseudo(comp.selectedLayers[0]);if(!pc)return "Selecione um MO Cloner Controls valido.";shape=Math.max(0,Math.min(3,Math.round(Number(shape)||0)));app.beginUndoGroup("MO Effector - Set Shape");try{setPseudo(pc,"Advanced Falloff","Effector Shape",shape);app.endUndoGroup();return "OK";}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function setFalloff(type,invert,power){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var pc=pseudo(comp.selectedLayers[0]);if(!pc)return "Selecione um MO Cloner Controls valido.";type=Math.max(0,Math.min(5,Math.round(Number(type)||0)));power=Math.max(.1,Math.min(10,Number(power)||2));app.beginUndoGroup("MO Effector - Set Falloff");try{setPseudo(pc,"Advanced Falloff","Falloff Type",type);setPseudo(pc,"Advanced Falloff","Falloff Invert",invert?1:0);setPseudo(pc,"Advanced Falloff","Falloff Power",power);app.endUndoGroup();return "OK";}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function addEffector(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Cloner Controls valido.";app.beginUndoGroup("MO Effector - Add Effector");var effector=null;try{ensureMultiControls(controller);var slot=0;for(var s=2;s<=4;s++){if(!linkedEffector(comp,controller,s)){slot=s;break;}}if(!slot)throw new Error("O limite e 4 Effectors por cloner.");var first=linkedEffector(comp,controller,1),position=first?first.property("ADBE Transform Group").property("ADBE Position").value:[comp.width/2,comp.height/2];effector=createVisualEffector(comp,controller,[position[0]+60*(slot-1),position[1]],"MO Effector "+slot);setLayerControl(controller,"MO Effector "+slot,effector);setSlider(controller,"MO Effector Count",slot);var clones=clonesFor(comp,controller),mode=controllerMode(controller,clones);for(var i=0;i<clones.length;i++)applyExpressions(clones[i],controller,i+1,mode,true);app.endUndoGroup();return "OK:"+slot;}catch(err){try{if(effector)effector.remove();}catch(cleanup){}app.endUndoGroup();return "Erro: "+err.toString();}}
  function removeLastEffector(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Cloner Controls valido.";app.beginUndoGroup("MO Effector - Remove Effector");try{ensureMultiControls(controller);for(var slot=4;slot>=2;slot--){var effector=linkedEffector(comp,controller,slot);if(effector){controllerEffect(controller,"MO Effector "+slot).property(1).setValue(0);setSlider(controller,"MO Effector Count",slot-1);effector.remove();app.endUndoGroup();return "OK:"+slot;}}app.endUndoGroup();return "Erro: Nao ha Effector adicional para remover.";}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function setCombineMode(mode){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Cloner Controls valido.";mode=Math.max(0,Math.min(3,Math.round(Number(mode)||0)));app.beginUndoGroup("MO Effector - Combine Mode");try{ensureMultiControls(controller);setSlider(controller,"MO Combine Mode",mode);app.endUndoGroup();return "OK";}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}}
  function setGlobePro(distribution,rings,seed,width,height,perspective,twist,orient,orientOffset){
    var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";
    var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Cloner Controls valido.";var clones=clonesFor(comp,controller),mode=controllerMode(controller,clones);if(mode!=="globe")return "Globe Pro requer um sistema criado no modo Globe.";
    distribution=Math.max(0,Math.min(2,Math.round(Number(distribution)||0)));rings=Math.max(2,Math.min(100,Math.round(Number(rings)||8)));seed=Math.max(0,Math.min(10000,Math.round(Number(seed)||1)));width=Math.max(1,Math.min(1000,Number(width)||100));height=Math.max(1,Math.min(1000,Number(height)||100));perspective=Math.max(-100,Math.min(100,Number(perspective)||0));twist=Math.max(-3600,Math.min(3600,Number(twist)||0));orientOffset=Math.max(-3600,Math.min(3600,Number(orientOffset)||0));
    app.beginUndoGroup("MO Effector - Globe Pro");try{setPseudo(pc,"Globe Pro","Globe Distribution",distribution);setPseudo(pc,"Globe Pro","Globe Rings",rings);setPseudo(pc,"Globe Pro","Globe Seed",seed);setPseudo(pc,"Globe Pro","Globe Width",width);setPseudo(pc,"Globe Pro","Globe Height",height);setPseudo(pc,"Globe Pro","Globe Perspective",perspective);setPseudo(pc,"Globe Pro","Globe Twist",twist);setPseudo(pc,"Globe Pro","Globe Surface Orient",orient?1:0);setPseudo(pc,"Globe Pro","Globe Orient Offset",orientOffset);for(var i=0;i<clones.length;i++)applyExpressions(clones[i],controller,i+1,mode,true);app.endUndoGroup();return "OK:"+clones.length;}catch(err){app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function collectPresetValues(group,values){for(var i=1;i<=group.numProperties;i++){var item=group.property(i);if(item.numProperties&&item.numProperties>0)collectPresetValues(item,values);else{try{var value=item.value;if(typeof value==="number"||value instanceof Array)values[item.name]=value;}catch(ignore){}}}}
  function selectedController(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return {error:"Abra uma composicao."};if(comp.selectedLayers.length!==1)return {error:"Selecione o MO Cloner Controls."};var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return {error:"Selecione um MO Cloner Controls valido."};return {comp:comp,controller:controller,pc:pc};}
  function getPresetData(){
    var selected=selectedController();if(selected.error)return "Erro: "+selected.error;var comp=selected.comp,controller=selected.controller,pc=selected.pc,data={format:1,values:{},effectors:[]};
    try{collectPresetValues(pc,data.values);var combine=controllerEffect(controller,"MO Combine Mode");data.combineMode=combine?combine.property(1).value:0;var origin=[data.values["Cloner Position X"]||0,data.values["Cloner Position Y"]||0];ensureMultiControls(controller);var countControl=controllerEffect(controller,"MO Effector Count"),count=Math.max(1,Math.min(4,Math.round(countControl?countControl.property(1).value:1)));for(var slot=1;slot<=count;slot++){var effector=linkedEffector(comp,controller,slot);if(!effector)continue;var transform=effector.property("ADBE Transform Group"),position=transform.property("ADBE Position").value;data.effectors.push({x:position[0]-origin[0],y:position[1]-origin[1],scale:transform.property("ADBE Scale").value,rotation:transform.property("ADBE Rotate Z").value,opacity:transform.property("ADBE Opacity").value});}return "OK:"+encodeURIComponent(JSON.stringify(data));}catch(err){return "Erro: "+err.toString();}
  }
  function applyPresetData(encoded){
    var selected=selectedController();if(selected.error)return "Erro: "+selected.error;var comp=selected.comp,controller=selected.controller,pc=selected.pc,data;
    try{data=JSON.parse(decodeURIComponent(String(encoded)));}catch(parseError){return "Erro: Preset invalido.";}if(!data||!data.values)return "Erro: Preset sem parametros.";
    app.beginUndoGroup("MO Effector - Apply Preset");
    try{
      for(var name in data.values)if(data.values.hasOwnProperty(name)&&name!=="Cloner Mode"){var param=findParam(pc,name);if(param)param.setValue(data.values[name]);}
      ensureMultiControls(controller);if(data.combineMode!==undefined)setSlider(controller,"MO Combine Mode",Math.max(0,Math.min(3,Math.round(data.combineMode))));
      if(data.effectors&&data.effectors.length){var desired=Math.max(1,Math.min(4,data.effectors.length)),origin=[pseudoParam(pc,"Formation","Cloner Position X").value,pseudoParam(pc,"Formation","Cloner Position Y").value];for(var slot=1;slot<=desired;slot++){var effector=linkedEffector(comp,controller,slot);if(!effector){effector=createVisualEffector(comp,controller,origin,slot===1?"MO Effector":"MO Effector "+slot);setLayerControl(controller,effectorControlName(slot),effector);}var saved=data.effectors[slot-1],transform=effector.property("ADBE Transform Group");transform.property("ADBE Position").setValue([origin[0]+saved.x,origin[1]+saved.y]);transform.property("ADBE Scale").setValue(saved.scale);transform.property("ADBE Rotate Z").setValue(saved.rotation);transform.property("ADBE Opacity").setValue(saved.opacity);}for(var removeSlot=4;removeSlot>desired;removeSlot--){var extra=linkedEffector(comp,controller,removeSlot);if(extra){controllerEffect(controller,effectorControlName(removeSlot)).property(1).setValue(0);extra.remove();}}setSlider(controller,"MO Effector Count",desired);}
      for(var layerIndex=1;layerIndex<=comp.numLayers;layerIndex++)comp.layer(layerIndex).selected=false;controller.selected=true;app.endUndoGroup();var syncResult=syncCloneCount();return syncResult.indexOf("OK:")===0?"OK":"Erro: O preset foi aplicado, mas o Sync falhou: "+syncResult;
    }catch(err){app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function selectPathGuide(){var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";var controller=comp.selectedLayers[0],control=controllerEffect(controller,"MO Path");if(!pseudo(controller)||!control)return "Este sistema nao possui um Path Cloner.";var index=Math.round(control.property(1).value),path=index>0?comp.layer(index):null;if(!path)return "MO Cloner Path nao foi encontrado.";for(var i=1;i<=comp.numLayers;i++)comp.layer(i).selected=false;path.selected=true;return "OK";}
  function hexColor(value){var text=String(value||"").replace("#","");if(text.length!==6)throw new Error("Cor hexadecimal invalida.");return [parseInt(text.substring(0,2),16)/255,parseInt(text.substring(2,4),16)/255,parseInt(text.substring(4,6),16)/255,1];}
  function setColorEffector(enabled,baseColor,effectorColor,amount,randomAmount,seed,depthEnabled,backColor,frontColor){
    var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";
    if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";
    var controller=comp.selectedLayers[0],pc=pseudo(controller);if(!pc)return "Selecione um MO Cloner Controls valido.";
    amount=Math.max(0,Math.min(100,Number(amount)||0));randomAmount=Math.max(0,Math.min(100,Number(randomAmount)||0));seed=Math.max(0,Math.min(10000,Math.round(Number(seed)||0)));
    app.beginUndoGroup("MO Effector - Color");
    try{
      setPseudo(pc,"Color Effector","Enable Color",enabled?1:0);setPseudo(pc,"Color Effector","Base Color",hexColor(baseColor));setPseudo(pc,"Color Effector","Effector Color",hexColor(effectorColor));setPseudo(pc,"Color Effector","Color Amount",amount);setPseudo(pc,"Color Effector","Random Color",randomAmount);setPseudo(pc,"Color Effector","Color Seed",seed);setPseudo(pc,"Color Effector","Globe Depth Color",depthEnabled?1:0);setPseudo(pc,"Color Effector","Globe Back Color",hexColor(backColor));setPseudo(pc,"Color Effector","Globe Front Color",hexColor(frontColor));
      var clones=clonesFor(comp,controller),mode=controllerMode(controller,clones);for(var i=0;i<clones.length;i++)applyColorExpression(clones[i],mode);
      app.endUndoGroup();return "OK:"+clones.length;
    }catch(err){app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function upgradeEffectorGuide(){
    var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";
    if(comp.selectedLayers.length!==1)return "Selecione o MO Controller.";
    var controller=comp.selectedLayers[0],pc=pseudo(controller),effect=controllerEffect(controller,"MO Effector");
    if(!pc||!effect)return "Selecione um MO Controller valido.";
    controller.name="MO Cloner Controls";ensureMultiControls(controller);
    var oldEffector=comp.layer(Math.round(effect.property(1).value));if(!oldEffector)return "MO Effector nao foi encontrado.";
    app.beginUndoGroup("MO Effector - Upgrade Visual Guide");var visual=null;
    try{
      var clones=clonesFor(comp,controller),mode=controllerMode(controller,clones);
      var oldTransform=oldEffector.property("ADBE Transform Group"),position=oldTransform.property("ADBE Position").value;
      visual=createVisualEffector(comp,controller,position);
      visual.property("ADBE Transform Group").property("ADBE Scale").setValue(oldTransform.property("ADBE Scale").value);
      visual.property("ADBE Transform Group").property("ADBE Rotate Z").setValue(oldTransform.property("ADBE Rotate Z").value);
      setLayerControl(controller,"MO Effector",visual);
      for(var i=0;i<clones.length;i++)applyExpressions(clones[i],controller,i+1,mode,true);
      oldEffector.remove();app.endUndoGroup();return "OK:upgraded";
    }catch(err){try{if(visual)visual.remove();}catch(cleanup){}app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function cleanSystem(){
    var comp=app.project.activeItem;if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";
    if(comp.selectedLayers.length!==1)return "Selecione o MO Cloner Controls.";
    var controller=comp.selectedLayers[0],pc=pseudo(controller),sourceEffect=controllerEffect(controller,"MO Source");
    if(!pc||!sourceEffect)return "Selecione um MO Cloner Controls valido.";
    ensureMultiControls(controller);var source=comp.layer(Math.round(sourceEffect.property(1).value)),sources=[],sourceCountControl=controllerEffect(controller,"MO Source Count"),sourceCount=Math.max(1,Math.min(8,Math.round(sourceCountControl?sourceCountControl.property(1).value:1)));for(var sourceSlot=1;sourceSlot<=sourceCount;sourceSlot++){var sourceControl=controllerEffect(controller,sourceControlName(sourceSlot)),linkedSource=sourceControl&&Math.round(sourceControl.property(1).value)>0?comp.layer(Math.round(sourceControl.property(1).value)):null;if(linkedSource)sources.push(linkedSource);}var effectors=[];for(var slot=1;slot<=4;slot++){var linked=linkedEffector(comp,controller,slot);if(linked)effectors.push(linked);}var pathControl=controllerEffect(controller,"MO Path"),pathGuide=pathControl&&Math.round(pathControl.property(1).value)>0?comp.layer(Math.round(pathControl.property(1).value)):null;
    app.beginUndoGroup("MO Effector - Clean System");
    try{
      var clones=clonesFor(comp,controller);for(var i=clones.length-1;i>=0;i--)clones[i].remove();
      for(var e=effectors.length-1;e>=0;e--)if(effectors[e]!==source)effectors[e].remove();if(pathGuide&&pathGuide!==source)pathGuide.remove();controller.remove();
      for(var j=1;j<=comp.numLayers;j++)comp.layer(j).selected=false;
      for(var s=0;s<sources.length;s++){sources[s].enabled=true;sources[s].selected=true;}if(!sources.length&&source){source.enabled=true;source.selected=true;}
      app.endUndoGroup();return "OK";
    }catch(err){app.endUndoGroup();return "Erro: "+err.toString();}
  }
  function create(rows,cols,sx,sy,mode,multiSource,sourceMode,sourceSeed){
    var comp=app.project.activeItem;
    if(!(comp&&comp instanceof CompItem))return "Abra uma composicao.";
    var selected=comp.selectedLayers;if(!multiSource&&selected.length!==1)return "Selecione uma unica camada 2D ou ative Multi-Source.";if(multiSource&&(selected.length<2||selected.length>8))return "Multi-Source requer entre 2 e 8 camadas selecionadas.";
    var sources=[],sourceStates=[],sourceNames=[];for(var si=0;si<selected.length;si++){if(selected[si].threeDLayer)return "O sistema suporta apenas camadas 2D.";sources.push(selected[si]);sourceStates.push(selected[si].enabled);sourceNames.push(selected[si].name);}var src=sources[0];
    var count=(mode==="grid"||mode==="scatter"?rows*cols:cols);
    if(count>400)return "O limite desta versao e 400 clones.";
    app.beginUndoGroup("MO Effector");var c=null,e=null,pathGuide=null,createdClones=[];
    try{
      c=comp.layers.addNull();c.name="MO Cloner Controls";c.label=9;c.property("ADBE Transform Group").property("ADBE Position").setValue([comp.width/2,comp.height/2]);
      e=createVisualEffector(comp,c,[comp.width/2,comp.height/2]);
      layerControl(c,"MO Effector",e);layerControl(c,"MO Source",src);ensureMultiControls(c);ensureSourceControls(c,sources);if(mode==="path"){pathGuide=createPathGuide(comp,c);layerControl(c,"MO Path",pathGuide);}
      var pc=c.property("ADBE Effect Parade").addProperty("Pseudo/MO Cloner Controls");if(!pc)throw new Error("MO Cloner Controls nao esta registrado. Reinicie o After Effects.");
      var sourcePosition=src.property("ADBE Transform Group").property("ADBE Position").value;
      setPseudo(pc,"Formation","Cloner Mode",mode==="linear"?1:mode==="radial"?2:mode==="scatter"?3:mode==="globe"?4:mode==="path"?5:0);setPseudo(pc,"Formation","Clone Count",count);setPseudo(pc,"Formation","Clone Size",100);setPseudo(pc,"Formation","Cloner Position X",sourcePosition[0]);setPseudo(pc,"Formation","Cloner Position Y",sourcePosition[1]);
      setPseudo(pc,"Grid","Grid Rows",rows);setPseudo(pc,"Grid","Grid Columns",cols);setPseudo(pc,"Grid","Spacing X",sx);setPseudo(pc,"Grid","Spacing Y",sy);
      setPseudo(pc,"Color Effector","Enable Color",1);
      setPseudo(pc,"Multi Source","Source Mode",Math.max(0,Math.min(1,Math.round(Number(sourceMode)||0))));setPseudo(pc,"Multi Source","Source Offset",0);setPseudo(pc,"Multi Source","Source Seed",Math.max(0,Math.min(10000,Math.round(Number(sourceSeed)||1))));
      if(mode==="globe")setPseudo(pc,"Globe","Globe Radius",sx);
      for(var ss=0;ss<sources.length;ss++){if(sources[ss].name.indexOf("MO Source ")!==0)sources[ss].name="MO Source "+(ss+1)+" - "+sources[ss].name;sources[ss].enabled=false;}
      for(var n=0;n<count;n++){
        var info=sourceForIndex(comp,c,pc,n+1),clone=info.layer.duplicate();createdClones.push(clone);clone.enabled=true;applyExpressions(clone,c,n+1,mode,true,info.slot);
      }
      app.endUndoGroup();return "OK";
    }catch(err){try{for(var ci=createdClones.length-1;ci>=0;ci--)createdClones[ci].remove();if(pathGuide)pathGuide.remove();if(e)e.remove();if(c)c.remove();for(var restore=0;restore<sources.length;restore++){sources[restore].enabled=sourceStates[restore];sources[restore].name=sourceNames[restore];}}catch(cleanup){}app.endUndoGroup();return "Erro: "+err.toString();}
  }
  return {create:create,createSourceShape:createSourceShape,syncCloneCount:syncCloneCount,centerCloner:centerCloner,setEffectorShape:setEffectorShape,setFalloff:setFalloff,addEffector:addEffector,removeLastEffector:removeLastEffector,setCombineMode:setCombineMode,setColorEffector:setColorEffector,setGlobePro:setGlobePro,getPresetData:getPresetData,applyPresetData:applyPresetData,selectPathGuide:selectPathGuide,upgradeEffectorGuide:upgradeEffectorGuide,cleanSystem:cleanSystem};
}());
