var more = {english: "More&#8230;"};

var DevPubsWindow = function (id){
    var box = document.createElement('div');
    var header = document.createElement('div');
    var contents = document.createElement('div');
    var footer = document.createElement('div');
    box.setAttribute('id', id+"Window");
    header.setAttribute('id', id+"Header");
    contents.setAttribute('id', id+"Body");
    footer.setAttribute('id', id+"Footer");
    box.appendChild(header);
    box.appendChild(contents);
    box.appendChild(footer);
    box.style.display = "none";
    document.body.appendChild(box);
    this.box = $(id+"Window");
    this.header = $(id+"Header");
    this.contents = $(id+"Body");
    this.footer = $(id+"Footer");
};

var QuickLook ={
    win: null,
    greyout: document.createElement('div'),
    createCloseBox: function(){
        var closeBox = document.createElement('img');
        closeBox.setAttribute('src', $('RESOURCES').content+'/Images/QuickLook/closebox.png');
        closeBox.setAttribute('id', 'QuickLookCloseBox');
        closeBox.style.display = "none";
        document.body.appendChild(closeBox);
        closeBox.observe("click", QuickLook.hide);
        return QuickLook.win.closeBox = closeBox;
    },
    createPediaLinksBox: function(){
        var pediaLinksBox = document.createElement('div');
        pediaLinksBox.setAttribute('id', "pediaLinks");
        var pediaLinks = $$(".pediaPrerequisites").concat($$(".pediaRelated"));
        for (i=0; i<pediaLinks.length; i++){
            pediaLinksBox.innerHTML += pediaLinks[i].innerHTML;
            pediaLinks[i].innerHTML = "";
        }
        return $$('h1')[0].insert({after:pediaLinksBox});
    },
    processLinks: function(){
        var links = $$('a');
        for (var i=0; i<links.length; i++){
            links[i].target= "_blank";
        }
    },
    init: function(){
        QuickLook.greyout.setAttribute("id", "QuickLookGreyout");
        QuickLook.greyout.style.display = "none";
        document.body.appendChild(QuickLook.greyout);
        QuickLook.win = new DevPubsWindow('QuickLook');
        QuickLook.createCloseBox();
        var article = document.createElement('object');
        QuickLook.win.contents.appendChild(article);
        return QuickLook.win;
    },
    setupWin: function(){
        var css=document.createElement("link");
        css.setAttribute("rel", "stylesheet");
        css.setAttribute("type", "text/css");
        css.setAttribute("href", $('RESOURCES').content+"/CSS/pediaQuickLook.css");
        $$("head")[0].appendChild(css);
        QuickLook.processLinks();
        QuickLook.createPediaLinksBox();
    },
    load: function(url){
        $$("#QuickLookBody object")[0].data = url;        
        QuickLook.greyout.show();
        QuickLook.win.box.appear({duration: .6});
        QuickLook.win.closeBox.appear({duration: .6});
        Pedia.hide("QuickLook.load()");
        return QuickLook.win;
    },
    hide: function(){
        QuickLook.win.box.switchOff({duration: .25});
        QuickLook.greyout.fade({duration: .4});
        QuickLook.win.closeBox.hide();
        return QuickLook.win;
    }
};

var Pedia = {
    visable: false,
    win: null,
    init: function(){// adds onclick's, returns the number of pedia links on the current page
        var pediaLinks = $$(".pediaLink a");
        for(var i=0; i<pediaLinks.length; i++ ){
            pediaLinks[i].observe("click", Pedia.click);
        }
        if (i>0){
            Pedia.win = new DevPubsWindow('pedia');
            QuickLook.init();
            document.observe("click", Pedia.hide);
        }
        return i;   
    },
    moreLink: function(){
        var moreLink=document.createElement('a');
        moreLink.setAttribute('id', 'peidaMore');
        moreLink.href="javascript:QuickLook.load(Pedia.visable.href)";
        moreLink.innerHTML = more.english;
        return moreLink;
    },
    position: function(){
        if($("bodyText")){
            var offset = -(Pedia.visable.positionedOffset().left - $("bodyText").getWidth() + 70);
        } else {
            offset = -(Pedia.visable.positionedOffset().left - document.body.getWidth() + 70); 
        }
        if (offset > 0){offset = -20;}
        return Pedia.win.box.clonePosition(Pedia.visable, {setWidth: false, setHeight: false, offsetLeft: offset, offsetTop: Pedia.visable.getHeight()});
    },
    click: function(event){//click event for pedia links, displays and returns pedia window
        event.stop();
        if (Pedia.visable){Pedia.visable.removeClassName('activePedia');}
        Pedia.visable=$(event.element());
        Pedia.win.header.innerHTML = Pedia.visable.siblings()[0].innerHTML.split("-p3d14-")[0];
        Pedia.win.contents.innerHTML = Pedia.visable.siblings()[0].innerHTML.split("-p3d14-")[1];
        Pedia.win.contents.insert(Pedia.moreLink());
        Pedia.position()
        Pedia.win.box.appear({duration: .6, queue: { position: 'end', scope: 'Pedia' }});
        Pedia.visable.addClassName('activePedia');
        window.onresize = Pedia.position;
    },
    hide: function(event){// hides the currenlty visable pedia window, returns boolean
        if(Pedia.visable && 
                event!="QuickLook.load()" &&
                event.element()!=Pedia.visable &&
                !event.element().descendantOf(Pedia.win.box)){
            Pedia.win.box.fade({duration: .25, queue: { position: 'start', scope: 'Pedia' }});
            Pedia.visable.removeClassName('activePedia');
            Pedia.visable = false;
        }
        return $$('activePedia').length == 0;
    }
};

loadPedia = function() {
    if (top.location.href!= window.location.href){
        return QuickLook.setupWin();
    } else {
        return Pedia.init();
    }
};