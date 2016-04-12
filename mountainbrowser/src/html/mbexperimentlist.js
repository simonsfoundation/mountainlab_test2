function MBExperimentList() {
    this.div=$('<div></div>');
    this.refresh=refresh;
    this.setExperiments=function(exp) {m_experiments=exp;}
    this.setBasePath=function(path) {m_base_path=path;}

    var m_experiments=[];
    var m_base_path='';

    function refresh() {
        console.log(JSON.stringify(m_experiments));
        this.div.empty();
        var ul=$('<ul></ul>');
        this.div.append(ul);
        for (var i=0; i<m_experiments.length; i++) {
            (function() {
                var E=m_experiments[i];
                E.basepath=m_base_path+'/'+E.basepath;
                var li=$('<li></li>'); ul.append(li);
                var a=$('<a href=#>'+E.exp_id+'</a>');
                a.click(function() {open_experiment(E);})
                li.append(a);
            })();
        }
    }

    function open_experiment(E) {
        console.log('OPEN :'+JSON.stringify(E));
        MB.openSortingResult(JSON.stringify(E));
    }
}

function refresh_experiment_list(X) {
    $('#experiment_list').empty();
    var T=$('<table></table>');
    $('#experiment_list').append(T);
    for (var j=0; j<X.length; j++) {
        E=X[j];
        E.description=E.description||'';
        aa='<a href="/open_experiment/'+E.exp_id+'">'+E.exp_id+'</a>';
        T.append('<tr><td>'+aa+'</td><td>'+E.description+'</td></tr>');
    }
}
