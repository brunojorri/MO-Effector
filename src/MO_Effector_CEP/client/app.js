(function(){
  var cs=new CSInterface(),status=document.getElementById('status'),mode='grid',globeInitialized=false,pathInitialized=false;
  function value(id){return Number(document.getElementById(id).value);}
  function setStatus(text,error){status.textContent=text;status.className=error?'error':'';}

  var presetStorageKey='moEffector.customPresets.v1';
  var presetBuiltins={
    organic:{name:'Organic Grid',data:{format:1,values:{'Noise Position X':55,'Noise Position Y':55,'Noise Scale':18,'Noise Rotation':22,'Noise Opacity':10,'Noise Speed':0.35,'Noise Seed':7}}},
    wave:{name:'Wave Cascade',data:{format:1,values:{'Step Position X':0,'Step Position Y':-180,'Step Scale':45,'Step Rotation':65,'Step Opacity':100,'Step Offset':0,'Step Falloff':100,'Step Randomise':0}}},
    pulse:{name:'Soft Effector Pulse',data:{format:1,values:{'Radius':340,'Inner Radius':45,'Strength':100,'Position X':0,'Position Y':-90,'Scale Amount':70,'Rotation Amount':35,'Target Opacity':20,'Falloff Type':4,'Falloff Power':2.4}}},
    orbit:{name:'Globe Orbit',data:{format:1,values:{'Globe Spin X':8,'Globe Spin Y':35,'Globe Spin Z':3,'Globe Back Scale':28,'Globe Front Scale':118,'Globe Back Opacity':18,'Globe Perspective':32,'Globe Twist':40,'Globe Width':100,'Globe Height':100}}},
    neon:{name:'Neon Depth',data:{format:1,values:{'Enable Color':1,'Base Color':[0.03,0.08,0.18,1],'Effector Color':[1,0.12,0.65,1],'Color Amount':100,'Random Color':12,'Color Seed':19,'Globe Depth Color':1,'Globe Back Color':[0.02,0.08,0.35,1],'Globe Front Color':[0.1,0.95,1,1]}}},
    fan:{name:'Stagger Fan',data:{format:1,values:{'Step Position X':150,'Step Position Y':-80,'Step Scale':65,'Step Rotation':160,'Step Opacity':35,'Step Offset':0,'Step Falloff':100,'Step Reverse':0,'Step Clamp':1,'Step Randomise':0}}}
  };
  function readCustomPresets(){try{return JSON.parse(localStorage.getItem(presetStorageKey)||'{}');}catch(ignore){return {};}}
  function writeCustomPresets(data){localStorage.setItem(presetStorageKey,JSON.stringify(data));}
  function option(group,value,label){var item=document.createElement('option');item.value=value;item.textContent=label;group.appendChild(item);}
  function refreshPresetSelect(preferred){var select=document.getElementById('presetSelect'),builtGroup=document.createElement('optgroup'),customGroup=document.createElement('optgroup'),custom=readCustomPresets();select.innerHTML='';builtGroup.label='MO BUILT-IN';for(var key in presetBuiltins)if(presetBuiltins.hasOwnProperty(key))option(builtGroup,'builtin:'+key,presetBuiltins[key].name);select.appendChild(builtGroup);customGroup.label='CUSTOM';var hasCustom=false;for(var name in custom)if(custom.hasOwnProperty(name)){option(customGroup,'custom:'+name,name);hasCustom=true;}if(hasCustom)select.appendChild(customGroup);if(preferred)select.value=preferred;}
  function selectedPresetData(){var selected=document.getElementById('presetSelect').value;if(selected.indexOf('builtin:')===0){var built=presetBuiltins[selected.substring(8)];return built?built.data:null;}if(selected.indexOf('custom:')===0)return readCustomPresets()[selected.substring(7)]||null;return null;}

  var moduleHeaders=document.querySelectorAll('.panel-module .section-title');
  for(var h=0;h<moduleHeaders.length;h++)moduleHeaders[h].onclick=function(){var panel=this.parentNode;if(panel.className.indexOf('collapsed')>=0)panel.className=panel.className.replace(' collapsed','');else panel.className+=' collapsed';};
  document.getElementById('credit').onclick=function(event){event.preventDefault();var url='https://www.instagram.com/brunojorri_work/';if(window.cep&&window.cep.util&&window.cep.util.openURLInDefaultBrowser)window.cep.util.openURLInDefaultBrowser(url);else window.open(url,'_blank');};

  refreshPresetSelect();
  document.getElementById('applyPreset').onclick=function(){var data=selectedPresetData();if(!data){setStatus('Choose a preset.',true);return;}var encoded=encodeURIComponent(JSON.stringify(data));setStatus('Applying preset...');cs.evalScript('MOEffector.applyPresetData('+JSON.stringify(encoded)+')',function(result){setStatus(result==='OK'?'Preset applied and clones synchronized.':result,result!=='OK');});};
  document.getElementById('savePreset').onclick=function(){var name=document.getElementById('presetName').value.replace(/^\s+|\s+$/g,'');if(!name){setStatus('Enter a custom preset name.',true);return;}var custom=readCustomPresets();if(custom[name]&&!window.confirm('Replace the custom preset "'+name+'"?'))return;setStatus('Reading Controller settings...');cs.evalScript('MOEffector.getPresetData()',function(result){if(result.indexOf('OK:')!==0){setStatus(result,true);return;}try{custom[name]=JSON.parse(decodeURIComponent(result.substring(3)));writeCustomPresets(custom);refreshPresetSelect('custom:'+name);setStatus('Custom preset saved: '+name);}catch(error){setStatus('Could not save preset: '+error.message,true);}});};
  document.getElementById('deletePreset').onclick=function(){var selected=document.getElementById('presetSelect').value;if(selected.indexOf('custom:')!==0){setStatus('Built-in presets cannot be deleted.',true);return;}var name=selected.substring(7);if(!window.confirm('Delete custom preset "'+name+'"?'))return;var custom=readCustomPresets();delete custom[name];writeCustomPresets(custom);refreshPresetSelect();setStatus('Custom preset deleted.');};

  var sourceButtons=document.querySelectorAll('[data-source]');
  for(var s=0;s<sourceButtons.length;s++)sourceButtons[s].onclick=function(){
    var kind=this.getAttribute('data-source'),size=value('sourceSize'),sides=value('polygonSides');
    if(size<10||size>2000){setStatus('Source size: 10 to 2000 px.',true);return;}
    if(kind==='polygon'&&(sides<3||sides>20)){setStatus('Polygon sides: 3 to 20.',true);return;}
    setStatus('Creating '+kind+' source...');
    cs.evalScript('MOEffector.createSourceShape("'+kind+'",'+size+','+sides+')',function(result){
      var ok=result.indexOf('OK:')===0;setStatus(ok?result.substring(3)+' created and selected.':result,!ok);
    });
  };
  document.getElementById('multiSource').onchange=function(){document.getElementById('sourceModeFields').className='source-mode'+(this.checked?' visible':'');};

  document.getElementById('create').onclick=function(){
    var rows=value('rows'),cols=value('cols'),count=(mode==='grid'||mode==='scatter')?rows*cols:cols,multi=document.getElementById('multiSource').checked,sourceMode=value('sourceMode'),sourceSeed=value('sourceSeed');
    if(!rows||!cols||count<1||count>400){setStatus('Use between 1 and 400 clones.',true);return;}
    if(sourceSeed<0||sourceSeed>10000){setStatus('Source Seed: 0 to 10000.',true);return;}
    setStatus('Creating cloner system...');
    cs.evalScript('MOEffector.create('+rows+','+cols+','+value('sx')+','+value('sy')+',"'+mode+'",'+(multi?'true':'false')+','+sourceMode+','+sourceSeed+')',function(result){setStatus(result==='OK'?'System created. Select MO Cloner Controls to edit.':result,result!=='OK');});
  };
  document.getElementById('sync').onclick=function(){
    setStatus('Synchronizing clones...');
    cs.evalScript('MOEffector.syncCloneCount()',function(result){setStatus(result.indexOf('OK:')===0?'Synchronized: '+result.substring(3)+' clones.':result,result.indexOf('OK:')!==0);});
  };
  document.getElementById('center').onclick=function(){
    setStatus('Centering cloner...');
    cs.evalScript('MOEffector.centerCloner()',function(result){setStatus(result==='OK'?'Cloner centered.':result,result!=='OK');});
  };
  document.getElementById('selectPath').onclick=function(){
    setStatus('Selecting Path guide...');
    cs.evalScript('MOEffector.selectPathGuide()',function(result){setStatus(result==='OK'?'Path selected. Edit its vertices in the canvas.':result,result!=='OK');});
  };
  document.getElementById('upgrade').onclick=function(){
    setStatus('Updating visual Effector and expressions...');
    cs.evalScript('MOEffector.upgradeEffectorGuide()',function(result){var ok=result.indexOf('OK:')===0;setStatus(ok?'Effector and clone expressions updated.':result,!ok);});
  };
  document.getElementById('clean').onclick=function(){
    if(!window.confirm('Remove this cloner system? The original source layer will be preserved and reactivated.'))return;
    setStatus('Cleaning cloner system...');
    cs.evalScript('MOEffector.cleanSystem()',function(result){setStatus(result==='OK'?'System removed. Source restored and selected.':result,result!=='OK');});
  };
  var effectorShapes=document.querySelectorAll('[data-effector-shape]');
  for(var e=0;e<effectorShapes.length;e++)effectorShapes[e].onclick=function(){
    var shape=Number(this.getAttribute('data-effector-shape'));setStatus('Changing Effector shape...');
    for(var k=0;k<effectorShapes.length;k++)effectorShapes[k].className='icon-tool';this.className='icon-tool shape-active';
    cs.evalScript('MOEffector.setEffectorShape('+shape+')',function(result){setStatus(result==='OK'?'Effector shape updated.':result,result!=='OK');});
  };
  document.getElementById('applyFalloff').onclick=function(){
    var type=value('falloffType'),power=value('falloffPower'),invert=document.getElementById('falloffInvert').checked;
    if(power<.1||power>10){setStatus('Falloff Power: 0.1 to 10.',true);return;}
    setStatus('Applying falloff...');cs.evalScript('MOEffector.setFalloff('+type+','+(invert?'true':'false')+','+power+')',function(result){setStatus(result==='OK'?'Falloff updated.':result,result!=='OK');});
  };
  document.getElementById('addEffector').onclick=function(){setStatus('Adding Effector...');cs.evalScript('MOEffector.addEffector()',function(result){var ok=result.indexOf('OK:')===0;setStatus(ok?'Effector '+result.substring(3)+' added. Move its guide layer.':result,!ok);});};
  document.getElementById('removeEffector').onclick=function(){setStatus('Removing last Effector...');cs.evalScript('MOEffector.removeLastEffector()',function(result){var ok=result.indexOf('OK:')===0;setStatus(ok?'Effector '+result.substring(3)+' removed.':result,!ok);});};
  document.getElementById('applyCombine').onclick=function(){var combine=value('combineMode');setStatus('Applying combine mode...');cs.evalScript('MOEffector.setCombineMode('+combine+')',function(result){setStatus(result==='OK'?'Combine mode updated.':result,result!=='OK');});};
  document.getElementById('depthColor').onchange=function(){document.getElementById('depthColors').className='color-pair color-depth'+(this.checked?' visible':'');};
  document.getElementById('applyColor').onclick=function(){
    var enabled=document.getElementById('enableColor').checked,depth=document.getElementById('depthColor').checked,amount=value('colorAmount'),random=value('randomColor'),seed=value('colorSeed');
    if(amount<0||amount>100||random<0||random>100||seed<0||seed>10000){setStatus('Color values are outside the allowed range.',true);return;}
    function quoted(id){return '"'+document.getElementById(id).value+'"';}
    setStatus('Applying Color Effector...');
    cs.evalScript('MOEffector.setColorEffector('+(enabled?'true':'false')+','+quoted('baseColor')+','+quoted('effectorColor')+','+amount+','+random+','+seed+','+(depth?'true':'false')+','+quoted('backColor')+','+quoted('frontColor')+')',function(result){var ok=result.indexOf('OK:')===0;setStatus(ok?'Color Effector applied to '+result.substring(3)+' clones.':result,!ok);});
  };
  document.getElementById('applyGlobePro').onclick=function(){
    var distribution=value('globeDistribution'),rings=value('globeRings'),seed=value('globeSeed'),width=value('globeWidth'),height=value('globeHeight'),perspective=value('globePerspective'),twist=value('globeTwist'),orient=document.getElementById('globeOrient').checked,offset=value('globeOrientOffset');
    if(rings<2||rings>100||seed<0||seed>10000||width<1||width>1000||height<1||height>1000||perspective<-100||perspective>100){setStatus('Globe Pro values are outside the allowed range.',true);return;}
    setStatus('Applying Globe Pro...');
    cs.evalScript('MOEffector.setGlobePro('+distribution+','+rings+','+seed+','+width+','+height+','+perspective+','+twist+','+(orient?'true':'false')+','+offset+')',function(result){var ok=result.indexOf('OK:')===0;setStatus(ok?'Globe Pro applied to '+result.substring(3)+' clones.':result,!ok);});
  };
  var modes=document.querySelectorAll('[data-mode]');
  for(var i=0;i<modes.length;i++)modes[i].onclick=function(){
    mode=this.getAttribute('data-mode');for(var j=0;j<modes.length;j++)modes[j].className='mode-button';this.className='mode-button active';
    var globe=mode==='globe',path=mode==='path',single=globe||path;document.getElementById('rowField').style.display=single?'none':'flex';document.getElementById('syField').style.display=single?'none':'flex';document.getElementById('sxField').style.display=path?'none':'flex';document.getElementById('pathHint').style.display=path?'block':'none';
    document.getElementById('colLabel').textContent=single?'Clone Count':'Columns / Count';document.getElementById('sxLabel').textContent=globe?'Globe Radius':'Spacing X / Radius';
    if(globe&&!globeInitialized){document.getElementById('cols').value=48;document.getElementById('sx').value=260;globeInitialized=true;}
    if(path&&!pathInitialized){document.getElementById('cols').value=18;pathInitialized=true;}
  };
}());
