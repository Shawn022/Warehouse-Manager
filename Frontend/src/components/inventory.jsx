import React, { useState, useMemo, useEffect } from 'react'

function downloadCSV(filename, rows) {
  if (!rows || rows.length === 0) return
  const headers = Object.keys(rows[0])
  const csv = [headers.join(','), ...rows.map(r => headers.map(h => `"${String(r[h] ?? '').replace(/"/g, '""')}"`).join(','))].join('\n')
  const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = filename
  a.click()
  URL.revokeObjectURL(url)
}

const Inventory = () => {
  const [items, setItems] = useState([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState(null)
  const [query, setQuery] = useState('')
  const [editing, setEditing] = useState(null) // item being edited

  useEffect(() => {
    let mounted = true
    const load = async () => {
      setLoading(true)
      setError(null)
      try {
        const res = await fetch('http://localhost:8080/inventory')
        if (!res.ok) throw new Error(`HTTP ${res.status} ${res.statusText}`)
        const data = await res.json()
        if (mounted) setItems(data)
      } catch (err) {
        if (mounted) setError(err.message || 'Failed to load')
      } finally {
        if (mounted) setLoading(false)
      }
    }
    load()
    return () => { mounted = false }
  }, [])

  const filtered = useMemo(() => {
    const q = query.trim().toLowerCase()
    if (!q) return items
    return items.filter(i => (i.sku || '').toLowerCase().includes(q) || (i.name || '').toLowerCase().includes(q) || ((i.location || '')).toLowerCase().includes(q))
  }, [items, query])

  async function addItem(item) {
    //setItems(prev => [{ ...item }, ...prev])
    item.id = Number(item.sku.split('-')[1]);
    try{
      const res= await fetch('http://localhost:8080/addItem',{
        method:'POST',
        mode:'cors',
        headers:{'Content-Type':'application/json'},
        body:JSON.stringify(item)
      })
      if(!res.ok) throw new Error(`HTTP ${res.status} ${res.statusText}`);
      const data= await res.json()
      //setItems(prev => [{ ...item }, ...prev])
      try{
        setLoading(true)
        const r=await fetch('http://localhost:8080/inventory')
        const data = await r.json()
        setItems(data);
        setLoading(false)
      }
      catch(e){
        setError(e.message || 'Failed to refresh')
      } 
    }
    catch(err){
      console.error('Add item failed:',err);
      //reloading the items
      
    }
  }

  async function updateItem(updated) {
    try{
      const res=await fetch('http://localhost:8080/updateItem',{
        method:'POST',
        mode:'cors',
        headers:{'Content-Type':'application/json'},
        body:JSON.stringify(updated)
      })
      if(!res.ok)throw new Error(`HTTP ${res.status} ${res.statusText}`);
      const data=await res.json();
      setItems(prev => prev.map(i => i.sku ===updated.sku ? updated: i))
    }
    catch(err){
      console.error('Update item Failed:',err);
      setError(err.message || 'Update failed');
      //reloading the items
      setLoading(true)
      try{
        const r=await fetch('http://localhost:8080/inventory')
        const data = await r.json()
        setItems(data);
      }
      catch(e){
        setError(e.message || 'Failed to refresh')
      }
    }
  }

  async function removeItem(id) {
    try {
      console.debug('Sending DELETE POST to server for id=', id)
      const res = await fetch('http://localhost:8080/deleteItem', {
        method: 'POST',
        mode: 'cors',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ key: id })
      })
      console.debug('Raw response', res.status, res.statusText)
      const text = await res.text()
      let resp = {};
      try { resp = JSON.parse(text) } catch (e) { console.debug('Non-JSON response:', text) }

      if (res.ok && resp.status === 'Item deleted') {
        console.debug('Delete confirmed by server:', resp)
        setItems(prev => prev.filter(i => i.id !== id))
      } else {
        console.warn('Delete not confirmed; server response:', res.status, resp, text)
        // fallback: reload list
        setLoading(true)
        try {
          const r = await fetch('http://localhost:8080/inventory')
          const data = await r.json()
          setItems(data)
        } catch (e) {
          setError(e.message || 'Failed to refresh')
        } finally {
          setLoading(false)
        }
      }
    } catch (err) {
      console.error('Delete request failed:', err)
      setError(err.message || 'Delete failed')
    }
  }

  if (loading) return <div className='loader'></div>
  if (error) return <div className="text-red-600">Error loading inventory: {error}</div>

  return (
    <div>
      <div className="flex items-center justify-between">
        <h2 className="text-4xl font-bold">Inventory</h2>
        <div className="flex gap-2">
          <button className="bg-gray-200 px-3 py-1 rounded" onClick={() => downloadCSV('inventory.csv', items)}>Export CSV</button>
          <button className="bg-green-600 text-white px-3 py-1 rounded" onClick={() => setEditing({ sku: '', name: '', quantity: 0, price: 0, reorder: 0 })}>Add Item</button>
        </div>
      </div>

      <div className="mt-4">
        <input placeholder="Search SKU, name" value={query} onChange={e => setQuery(e.target.value)} className="p-2 border rounded w-full md:w-1/2" />
      </div>

      <div className="item-container p-3 bg-green-50 rounded-lg mt-4">
        {filtered.length === 0 ? (
          <div className="p-4 text-gray-600">No items found.</div>
        ) : (
          filtered.map(item => (
            <div key={item.sku} className="item p-3 bg-white rounded mb-3 flex justify-between items-center">
              <div>
                <div className="font-semibold">{item.sku} — {item.name}</div>
                <div className="text-sm text-gray-500">Qty: {item.quantity} • Price/Unit: {item.price} • Reorder: {item.reorder}</div>
              </div>
              <div className="flex gap-2">
                <button className="bg-blue-500 text-white px-3 py-1 rounded" cursor="pointer" onClick={() => setEditing(item)}>Edit</button>
                <button className="bg-red-500 text-white px-3 py-1 rounded" cursor="pointer" onClick={() => removeItem(item.id)}>Delete</button>
              </div>
            </div>
          ))
        )}
      </div>

      {editing && (
        <div className="fixed inset-0 flex items-center justify-center bg-black/40">
          <div className="bg-white p-6 rounded w-11/12 md:w-1/2">
            <h3 className="text-xl font-semibold mb-3">{editing.sku ? 'Edit Item' : 'Add Item'}</h3>
            <div className="grid grid-cols-1 gap-3">
              <label className="text-sm">SKU</label>
              <input value={editing.sku} onChange={e => setEditing({ ...editing, sku: e.target.value })} className="p-2 border rounded" disabled={!!editing.sku && items.some(i => i.sku === editing.sku && i !== editing)} />

              <label className="text-sm">Name</label>
              <input value={editing.name} onChange={e => setEditing({ ...editing, name: e.target.value })} className="p-2 border rounded" />

              <label className="text-sm">Quantity</label>
              <input type="number" value={editing.quantity} onChange={e => setEditing({ ...editing, quantity: Number(e.target.value) })} className="p-2 border rounded" />

              <label className="text-sm">Price per Unit</label>
              <input type='number' value={editing.price} onChange={e => setEditing({ ...editing, price: Number(e.target.value) })} className="p-2 border rounded" />

              <label className="text-sm">Reorder Point</label>
              <input type="number" value={editing.reorder} onChange={e => setEditing({ ...editing, reorder: Number(e.target.value) })} className="p-2 border rounded" />

              <div className="flex justify-end gap-2 mt-3">
                <button className="px-3 py-1 rounded bg-gray-200" onClick={() => setEditing(null)}>Cancel</button>
                <button className="px-3 py-1 rounded bg-green-600 text-white" onClick={() => {
                  if (!editing.sku || !editing.name) return alert('SKU and name required')
                  const exists = items.some(i => i.sku === editing.sku)
                  if (exists) updateItem(editing)
                  else addItem(editing)
                  setEditing(null)
                }}>Save</button>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  )
}

export default Inventory
