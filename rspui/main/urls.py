from django.conf.urls import patterns, include, url
from django.contrib import admin

urlpatterns = patterns('',
    url(r'^$', 'main.views.index'),
    url(r'^on/', 'main.views.on', name='on'),
    url(r'^off/', 'main.views.off', name='off'),
    url(r'^automatic/', 'main.views.automatic', name='automatic'),
    url(r'^status/', 'main.views.status', name='status'),
)
