var clusters=[];
function setup_clusters() {
    var cluster_numbers=_CP.clusterNumbers();
    cluster_numbers=JSON.parse(cluster_numbers);
    for (var i in cluster_numbers) {
        var C=new _Cluster(cluster_numbers[i]);
        clusters.push(C);
    }
}

function _Cluster(num) {
    this.metric=function(metric_name) {
        return _CP.metric(num,metric_name);
    }
    this.addTag=function(tag_name) {
        console.log('ADDING TAG: '+num+' '+tag_name);
        _CP.addTag(num,tag_name);
    }
    this.removeTag=function(tag_name) {
        _CP.removeTag(num,tag_name);
    }
}

var console={
    log:function(msg) {
        if ((typeof msg)!='string')
            msg=JSON.stringify(msg);
        _CP.log(msg);
    }
};

setup_clusters();
