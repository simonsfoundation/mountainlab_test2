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
