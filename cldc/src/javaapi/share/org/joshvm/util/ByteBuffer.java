/*
 * Copyright (C) Max Mu
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please visit www.joshvm.org if you need additional information or
 * have any questions.
 */
package org.joshvm.util;

/**
 * A byte-array backed ByteBuffer. It could be regarded as a customized version 
 * of java.nio.ByteBuffer
 *
 * 
 * <p> A buffer is a linear, finite sequence of elements of a specific
 * primitive type.  Aside from its content, the essential properties of a
 * buffer are its capacity, limit, and position: </p>
 *
 * <blockquote>
 *
 *   <p> A buffer's <i>capacity</i> is the number of elements it contains.  The
 *   capacity of a buffer is never negative and never changes.  </p>
 *
 *   <p> A buffer's <i>limit</i> is the index of the first element that should
 *   not be read or written.  A buffer's limit is never negative and is never
 *   greater than its capacity.  </p>
 *
 *   <p> A buffer's <i>position</i> is the index of the next element to be
 *   read or written.  A buffer's position is never negative and is never
 *   greater than its limit.  </p>
 *
 * </blockquote>
 *
 * <p> ByteBuffer is a buffer with elements in byte type
 *
 *
 * <h4> Transferring data </h4>
 *
 * <p> Each subclass of this class defines two categories of <i>get</i> and
 * <i>put</i> operations: </p>
 *
 * <blockquote>
 *
 *   <p> <i>Relative</i> operations read or write one or more elements starting
 *   at the current position and then increment the position by the number of
 *   elements transferred.  If the requested transfer exceeds the limit then a
 *   relative <i>get</i> operation throws a {@link BufferUnderflowException}
 *   and a relative <i>put</i> operation throws a {@link
 *   BufferOverflowException}; in either case, no data is transferred.  </p>
 *
 *   <p> <i>Absolute</i> operations take an explicit element index and do not
 *   affect the position.  Absolute <i>get</i> and <i>put</i> operations throw
 *   an {@link IndexOutOfBoundsException} if the index argument exceeds the
 *   limit.  </p>
 *
 * </blockquote>
 *
 * <p> Data may also, of course, be transferred in to or out of a buffer by the
 * I/O operations of an appropriate channel, which are always relative to the
 * current position.
 *
 *
 * <h4> Marking and resetting </h4>
 *
 * <p> A buffer's <i>mark</i> is the index to which its position will be reset
 * when the {@link #reset reset} method is invoked.  The mark is not always
 * defined, but when it is defined it is never negative and is never greater
 * than the position.  If the mark is defined then it is discarded when the
 * position or the limit is adjusted to a value smaller than the mark.  If the
 * mark is not defined then invoking the {@link #reset reset} method causes an
 * {@link InvalidMarkException} to be thrown.
 *
 *
 * <h4> Invariants </h4>
 *
 * <p> The following invariant holds for the mark, position, limit, and
 * capacity values:
 *
 * <blockquote>
 *     <tt>0</tt> <tt>&lt;=</tt>
 *     <i>mark</i> <tt>&lt;=</tt>
 *     <i>position</i> <tt>&lt;=</tt>
 *     <i>limit</i> <tt>&lt;=</tt>
 *     <i>capacity</i>
 * </blockquote>
 *
 * <p> A newly-created buffer always has a position of zero and a mark that is
 * undefined.  The initial limit may be some value that depends upon the manner
 * in which it is constructed.  Each element of a newly-allocated buffer is 
 * initialized to zero.
 *
 *
 * <h4> Clearing, flipping, and rewinding </h4>
 *
 * <p> In addition to methods for accessing the position, limit, and capacity
 * values and for marking and resetting, this class also defines the following
 * operations upon buffers:
 *
 * <ul>
 *
 *   <li><p> {@link #clear} makes a buffer ready for a new sequence of
 *   channel-read or relative <i>put</i> operations: It sets the limit to the
 *   capacity and the position to zero.  </p></li>
 *
 *   <li><p> {@link #flip} makes a buffer ready for a new sequence of
 *   channel-write or relative <i>get</i> operations: It sets the limit to the
 *   current position and then sets the position to zero.  </p></li>
 *
 *   <li><p> {@link #rewind} makes a buffer ready for re-reading the data that
 *   it already contains: It leaves the limit unchanged and sets the position
 *   to zero.  </p></li>
 *
 * </ul>
 *
 *
 * <h4> Read-only buffers </h4>
 *
 * <p> Every buffer is readable, but not every buffer is writable.  The
 * mutation methods of each buffer class are specified as <i>optional
 * operations</i> that will throw a {@link ReadOnlyBufferException} when
 * invoked upon a read-only buffer.  A read-only buffer does not allow its
 * content to be changed, but its mark, position, and limit values are mutable.
 * Whether or not a buffer is read-only may be determined by invoking its
 * {@link #isReadOnly isReadOnly} method.
 *
 *
 * <h4> Thread safety </h4>
 *
 * <p> Buffers are not safe for use by multiple concurrent threads.  If a
 * buffer is to be used by more than one thread then access to the buffer
 * should be controlled by appropriate synchronization.
 *
 *
 * <h4> Invocation chaining </h4>
 *
 * <p> Methods in this class that do not otherwise have a value to return are
 * specified to return the buffer upon which they are invoked.  This allows
 * method invocations to be chained; for example, the sequence of statements
 *
 * <blockquote><pre>
 * b.flip();
 * b.position(23);
 * b.limit(42);</pre></blockquote>
 *
 * can be replaced by the single, more compact statement
 *
 * <blockquote><pre>
 * b.flip().position(23).limit(42);</pre></blockquote>
 *
 *
 *
 * @author Ferenc Hechler (ferenc@hechler.de)
 * @author Joerg Jahnke (joergjahnke@users.sourceforge.net)
 * @author Max Mu
 */
public final class ByteBuffer {

    /**
     * the byte array backing this buffer
     */
    private final byte[] buf;
    /**
     * offset to the first byte to use
     */
    private int ofs;
    /**
     * current buffer position
     */
    private int pos;
    /**
     * current available buffer size
     */
    private int siz;
    /**
     * a marker inside the buffer
     */
    private int mrk = -1;

    /**
     * Create a new ByteBuffer from an existing array
     *
     * @param buf   the array backing this buffer
     */
    public ByteBuffer(final byte[] buf) {
        this(buf, 0, 0, (buf == null ? 0 : buf.length));
    }

    /**
     * Create a new ByteBuffer from an existing array
     *
     * @param buf   the array backing this buffer
     * @param ofs   offset to the first byte to use
     * @param pos   current buffer position
     * @param siz   index of the first element in back array that should not be read or written.
     */
    public ByteBuffer(final byte[] buf, final int ofs, final int pos, final int siz) {
    	if (ofs < 0 || pos < 0 || siz < 0 || buf.length < ofs + pos + siz) {
			throw new IllegalArgumentException();
		}
        this.buf = buf;
        this.ofs = ofs;
        this.pos = pos;
        this.siz = siz;
    }

    /**
     * Returns this buffer's position. </p>
     *
     * @return  The position of this buffer
     */
    public int position() {
        return pos - ofs;
    }

    /**
     * Sets this buffer's position.  If the mark is defined and larger than the
     * new position then it is discarded. </p>
     *
     * @param  newPosition
     *         The new position value; must be non-negative
     *         and no larger than the current limit
     *
     * @return  This buffer
     *
     * @throws  IllegalArgumentException
     *          If the preconditions on <tt>newPosition</tt> do not hold
     */
    public ByteBuffer position(final int newPosition) {
		if ((newPosition > siz) || (newPosition < 0)) {
            throw new IllegalArgumentException();
		}

		pos = newPosition;
		
        if (mrk > pos) {
			mrk = -1;
        }
        return this;
    }

    /**
     * <p>Relative get method. Reads the byte at this buffer's current position, 
     * and then increments the position.</p>
     *
     * @param  newPosition
     *         The new position value; must be non-negative
     *         and no larger than the current limit
     *
     * @return  The byte at the buffer's current position
     *
     * @throws  BufferUnderflowException
     *          If the buffer's current position is not smaller than its limit
     */    
    public byte get() {
        if (pos >= siz) {
			throw new BufferUnderflowException();
		}
        return buf[pos++];
    }

    /**
     * Returns the number of elements between the current position and the
     * limit. </p>
     *
     * @return  The number of elements remaining in this buffer
     */
    public int remaining() {
        return siz - pos;
    }

    
    /**
     * <p>Absolute get method. Reads the byte at the given index.</p>
     *
     * @param  index
     *         The index from which the byte will be read
     *
     * @return  The byte at the given index
     *
     * @throws  IndexOutOfBoundsException
     *          If index is negative or not smaller than the buffer's limit
     */  
    public byte get(final int index) {
    	if (index < 0 || index >= limit()) {
			throw new IndexOutOfBoundsException();
		}
        return buf[index + ofs];
    }

	/**
     * <p>Allocates a new byte buffer. <br>
     *    The new buffer's position will be zero, its limit will be its capacity, 
     *    its mark will be undefined, and each of its elements will be initialized 
     *    to zero. It will have a backing array, and its array offset will be zero.</p>
     *
     * @param  capacity
     *         The new buffer's capacity, in bytes
     *
     * @return  The new byte buffer
     *
     * @throws  IllegalArgumentException
     *          If the capacity is a negative integer
     */  
    public static ByteBuffer allocate(final int capacity) {
    	if (capacity < 0) {
			throw new IllegalArgumentException();
    	}
        return new ByteBuffer(new byte[capacity]);
    }

	/**
     * <p>Allocates a new direct byte buffer. (currently implemented as same as<code>allocate</code>)
     *    <br>
     *    The new buffer's position will be zero, its limit will be its capacity, 
     *    its mark will be undefined, and each of its elements will be initialized 
     *    to zero. It will have a backing array, and its array offset will be zero.</p>
     *
     * @param  capacity
     *         The new buffer's capacity, in bytes
     *
     * @return  The new byte buffer
     *
     * @throws  IllegalArgumentException
     *          If the capacity is a negative integer
     */  
	public static ByteBuffer allocateDirect(final int size) {
		// TODO: Use native allocate buffer 
        return allocate(size);
    }

    /**
     * Creates a new byte buffer whose content is a shared subsequence of this buffer's content.<br>
     * The content of the new buffer will start at this buffer's current position. Changes to this 
     * buffer's content will be visible in the new buffer, and vice versa; the two buffers' position, 
     * limit, and mark values will be independent.<br>
     * The new buffer's position will be zero, its capacity and its limit will be the number of bytes
     * remaining in this buffer, and its mark will be undefined. The new buffer will be direct if, and
     * only if, this buffer is direct, and it will be read-only if, and only if, this buffer is read-only.
     *
     * @return  The new byte buffer
     */  
    public ByteBuffer slice() {
        return new ByteBuffer(buf, pos, pos, siz);
    }

    /**
     * Sets this buffer's limit.  If the position is larger than the new limit
     * then it is set to the new limit.  If the mark is defined and larger than
     * the new limit then it is discarded. </p>
     *
     * @param  newLimit
     *         The new limit value; must be non-negative
     *         and no larger than this buffer's capacity
     *
     * @return  This buffer
     *
     * @throws  IllegalArgumentException
     *          If the preconditions on <tt>newLimit</tt> do not hold
     */
    public ByteBuffer limit(final int newLimit) {
         if ((newLimit > buf.length) || (newLimit < 0)) {
            throw new IllegalArgumentException();
         }
		 
        siz = ofs + newLimit;
		if (pos > siz) {
			pos = siz;
		}
		if (mrk > siz) {
			mrk = -1;
		}

		return this;
    }

    /**
     * Relative bulk <i>get</i> method.
     *
     * <p> This method transfers bytes from this buffer into the given
     * destination array.  An invocation of this method of the form
     * <tt>src.get(outBuf)</tt> behaves in exactly the same way as the invocation
     *
     * <pre>
     *     src.get(outBuf, 0, outBuf.length) </pre>
     *
     * @return  This buffer
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than <tt>length</tt> bytes
     *          remaining in this buffer
     */
    public void get(final byte[] outBuf) {
        System.arraycopy(buf, pos, outBuf, 0, outBuf.length);
        pos += outBuf.length;
    }

    /**
     * Rewinds this buffer.  The position is set to zero and the mark is
     * discarded.
     *
     * <p> Invoke this method before a sequence of channel-write or <i>get</i>
     * operations, assuming that the limit has already been set
     * appropriately.  For example:
     *
     * <blockquote><pre>
     * out.write(buf);    // Write remaining data
     * buf.rewind();      // Rewind buffer
     * buf.get(array);    // Copy data into array</pre></blockquote>
     *
     * @return  This buffer
     */
    public ByteBuffer rewind() {
        pos = ofs;
		mrk = -1;
		return this;
    }

    /**
     * Returns this buffer's limit. </p>
     *
     * @return  The limit of this buffer
     */
    public int limit() {
        return siz - ofs;
    }

	/**
     * Returns this buffer's capacity. </p>
     *
     * @return  The capacity of this buffer
     */
    public final int capacity() {
        return buf.length - ofs;
    }

    /**
     * Tells whether or not this buffer is backed by an accessible
     * array.
     *
     * <p> If this method returns <tt>true</tt> then the {@link #array() array}
     * and {@link #arrayOffset() arrayOffset} methods may safely be invoked.
     * </p>
     *
     * @return  <tt>true</tt> if, and only if, this buffer
     *          is backed by an array and is not read-only
     *
     */
    public boolean hasArray() {
        return true;
    }

    /**
     * Returns the offset within this buffer's backing array of the first
     * element of the buffer&nbsp;&nbsp;<i>(optional operation)</i>.
     *
     * <p> If this buffer is backed by an array then buffer position <i>p</i>
     * corresponds to array index <i>p</i>&nbsp;+&nbsp;<tt>arrayOffset()</tt>.
     *
     * <p> Invoke the {@link #hasArray hasArray} method before invoking this
     * method in order to ensure that this buffer has an accessible backing
     * array.  </p>
     *
     * @return  The offset within this buffer's array
     *          of the first element of the buffer
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is backed by an array but is read-only
     *
     * @throws  UnsupportedOperationException
     *          If this buffer is not backed by an accessible array
     *
     */
    public int arrayOffset() {
        return ofs;
    }

    /* (non-Javadoc)
     * @see net.sf.andpdf.pdfviewer.ByteBuffer#array()
     */
    public byte[] array() {
        return buf;
    }

    /**
     * Flips this buffer.  The limit is set to the current position and then
     * the position is set to zero.  If the mark is defined then it is
     * discarded.
     *
     * <p> After a sequence of channel-read or <i>put</i> operations, invoke
     * this method to prepare for a sequence of channel-write or relative
     * <i>get</i> operations.  For example:
     *
     * <blockquote><pre>
     * buf.put(magic);    // Prepend header
     * in.read(buf);      // Read data into rest of buffer
     * buf.flip();        // Flip buffer
     * out.write(buf);    // Write header + data to channel</pre></blockquote>
     *
     * <p> This method is often used in conjunction with the {@link
     * ByteBuffer#compact compact} method when transferring data from one place
     * to another.  </p>
     *
     * @return  This buffer
     */
    public ByteBuffer flip() {
        siz = pos;
        pos = ofs;
		mrk = -1;
		return this;
    }

	/**
     * Compacts this buffer&nbsp;&nbsp;<i>(optional operation)</i>.
     *
     * <p> The bytes between the buffer's current position and its limit,
     * if any, are copied to the beginning of the buffer.  That is, the
     * byte at index <i>p</i>&nbsp;=&nbsp;<tt>position()</tt> is copied
     * to index zero, the byte at index <i>p</i>&nbsp;+&nbsp;1 is copied
     * to index one, and so forth until the byte at index
     * <tt>limit()</tt>&nbsp;-&nbsp;1 is copied to index
     * <i>n</i>&nbsp;=&nbsp;<tt>limit()</tt>&nbsp;-&nbsp;<tt>1</tt>&nbsp;-&nbsp;<i>p</i>.
     * The buffer's position is then set to <i>n+1</i> and its limit is set to
     * its capacity.  The mark, if defined, is discarded.
     *
     * <p> The buffer's position is set to the number of bytes copied,
     * rather than to zero, so that an invocation of this method can be
     * followed immediately by an invocation of another relative <i>put</i>
     * method. </p>
     *
     * <p> Invoke this method after writing data from a buffer in case the
     * write was incomplete.  The following loop, for example, copies bytes
     * from one channel to another via the buffer <tt>buf</tt>:
     *
     * <blockquote><pre>{@code
     *   buf.clear();          // Prepare buffer for use
     *   while (in.read(buf) >= 0 || buf.position != 0) {
     *       buf.flip();
     *       out.write(buf);
     *       buf.compact();    // In case of partial write
     *   }
     * }</pre></blockquote>
     *
     *
     * @return  This buffer
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer compact() {
        int rem = siz - pos;
        System.arraycopy(buf, pos, buf, ofs, rem);
        position(rem);
        limit(capacity());
        mrk = -1;
        return this;
	}

	public boolean isDirect() {
		return false;
	}

    /**
     * Creates a new byte buffer that shares this buffer's content.
     *
     * <p> The content of the new buffer will be that of this buffer.  Changes
     * to this buffer's content will be visible in the new buffer, and vice
     * versa; the two buffers' position, limit, and mark values will be
     * independent.
     *
     * <p> The new buffer's capacity, limit, position, and mark values will be
     * identical to those of this buffer.  The new buffer will be direct if,
     * and only if, this buffer is direct, and it will be read-only if, and
     * only if, this buffer is read-only.  </p>
     *
     * @return  The new byte buffer
     */
    public ByteBuffer duplicate() {
        return new ByteBuffer(buf, ofs, pos, siz);
    }

    /**
     * Wraps a byte array into a buffer.
     *
     * <p> The new buffer will be backed by the given byte array;
     * that is, modifications to the buffer will cause the array to be modified
     * and vice versa.  The new buffer's capacity and limit will be
     * <tt>array.length</tt>, its position will be zero, and its mark will be
     * undefined.  Its {@link #array </code>backing array<code>} will be the
     * given array, and its {@link #arrayOffset </code>array offset<code>} will
     * be zero.  </p>
     *
     * @param  array
     *         The array that will back this buffer
     *
     * @return  The new byte buffer
     */
    public static ByteBuffer wrap(final byte[] bytes) {
        return new ByteBuffer(bytes);
    }


    /**
     * Wraps a byte array into a buffer.
     *
     * <p> The new buffer will be backed by the given byte array;
     * that is, modifications to the buffer will cause the array to be modified
     * and vice versa.  The new buffer's capacity will be
     * <tt>array.length</tt>, its position will be <tt>offset</tt>, its limit
     * will be <tt>offset + length</tt>, and its mark will be undefined.  Its
     * {@link #array </code>backing array<code>} will be the given array, and
     * its {@link #arrayOffset </code>array offset<code>} will be zero.  </p>
     *
     * @param  array
     *         The array that will back the new buffer
     *
     * @param  offset
     *         The offset of the subarray to be used; must be non-negative and
     *         no larger than <tt>array.length</tt>.  The new buffer's position
     *         will be set to this value.
     *
     * @param  length
     *         The length of the subarray to be used;
     *         must be non-negative and no larger than
     *         <tt>array.length - offset</tt>.
     *         The new buffer's limit will be set to <tt>offset + length</tt>.
     *
     * @return  The new byte buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If the preconditions on the <tt>offset</tt> and <tt>length</tt>
     *          parameters do not hold
     */
	public static ByteBuffer wrap(final byte[] array, int offset, int length) {
		try {
        	return new ByteBuffer(array, 0, offset, offset + length);
		} catch (IllegalArgumentException e) {
			throw new IndexOutOfBoundsException();
		}
    }

    /**
     * Absolute get method for reading a char value.<br>
     * Reads two bytes at the given index, composing them into a char value
     * according to the current byte order.
     *
     * @param   index
     *          The index from which the bytes will be read
     *
     * @return  The char value at the given index
     *
     * @throws  IndexOutOfBoundsException
     *          If index is negative or not smaller than the buffer's limit, minus one
     */
    public char getChar(final int index) {
        // TODO: check current byteorder, assume BIG_ENDIAN
        int result = get(index) & 0xff;
        result = (result << 8) + (get(index + 1) & 0xff);
        return (char) result;
    }

    /**
     * Relative get method for reading a int value.<br>
     * Reads the next four bytes at this buffer's current position, composing
     * them into a int value according to the current byte order, and then 
     * increments the position by four.
     *
     * @return  The int value at the buffer's current position
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than two bytes remaining in this buffer
     */
    public int getInt() {
        // TODO: check current byteorder, assume BIG_ENDIAN
        final int pos_ = this.pos;
        final byte[] buf_ = this.buf;
        final int result = ((get() & 0xff) << 24) + ((get() & 0xff) << 16) + ((get() & 0xff) << 8) + (get() & 0xff);
        return result;
    }

    /**
     * Relative get method for reading a long value.<br>
     * Reads the next eight bytes at this buffer's current position, composing
     * them into a long value according to the current byte order, and then 
     * increments the position by eight.
     *
     * @return  The long value at the buffer's current position
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than two bytes remaining in this buffer
     */
    public long getLong() {
        // TODO: check current byteorder, assume BIG_ENDIAN
        long result = get() & 0xff;
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        result = (result << 8) + (get() & 0xff);
        return result;
    }


    /**
     * Relative get method for reading a char value.<br>
     * Reads the next two bytes at this buffer's current position, composing
     * them into a char value according to the current byte order, and then 
     * increments the position by two.
     *
     * @return  The char value at the buffer's current position
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than two bytes remaining in this buffer
     */
    public char getChar() {
        // TODO: check current byteorder, assume BIG_ENDIAN
        int result = get() & 0xff;
        result = (result << 8) + (get() & 0xff);
        return (char) result;
    }

	/**
	 * Relative get method for reading a short value.<br>
	 * Reads the next two bytes at this buffer's current position, composing
	 * them into a short value according to the current byte order, and then 
	 * increments the position by two.
	 *
	 * @return	The short value at the buffer's current position
	 *
	 * @throws	BufferUnderflowException
	 *			If there are fewer than two bytes remaining in this buffer
	 */
    public short getShort() {
        final int result = ((get() & 0xff) << 8) + (get() & 0xff);

        return (short)result;
    }

	/**
     * Absolute <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given byte into this buffer at the given
     * index. </p>
     *
     * @param  index
     *         The index at which the byte will be written
     *
     * @param  b
     *         The byte value to be written
     *
     * @return  This buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If <tt>index</tt> is negative
     *          or not smaller than the buffer's limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer put(final int index, final byte b) {
    	if (index < 0 || index >= siz) {
			throw new BufferOverflowException();
		}
        buf[index + ofs] = b;
		return this;
    }

	/**
     * Relative <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given byte into this buffer at the current
     * position, and then increments the position. </p>
     *
     * @param  b
     *         The byte to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If this buffer's current position is not smaller than its limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer put(final byte b) {
    	if (pos >= siz) {
			throw new BufferOverflowException();
		}
        buf[pos++] = b;
		return this;
    }

	/**
     * Relative <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given int value into this buffer at the current
     * position, and then increments the position by four. </p>
     *
     * @param  i
     *         The int value to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If this buffer's current position is not smaller than its limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer putInt(final int i) {
        put((byte) ((i >> 24) & 0xff));
        put((byte) ((i >> 16) & 0xff));
        put((byte) ((i >> 8) & 0xff));
        put((byte) (i & 0xff));
		return this;
    }

	/**
     * Relative <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given short value into this buffer at the current
     * position, and then increments the position by two. </p>
     *
     * @param  s
     *         The short value to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If this buffer's current position is not smaller than its limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer putShort(final short s) {
        put((byte) ((s >> 8) & 0xff));
        put((byte) (s & 0xff));
		return this;
    }

	/**
     * Clears this buffer.  The position is set to zero, the limit is set to
     * the capacity, and the mark is discarded.
     *
     * <p> Invoke this method before using a sequence of channel-read or
     * <i>put</i> operations to fill this buffer.  For example:
     *
     * <blockquote><pre>
     * buf.clear();     // Prepare buffer for reading
     * in.read(buf);    // Read data</pre></blockquote>
     *
     * <p> This method does not actually erase the data in the buffer, but it
     * is named as if it did because it will most often be used in situations
     * in which that might as well be the case. </p>
     *
     * @return  This buffer
     */
    public final ByteBuffer clear() {
        pos = ofs;
        siz = buf.length;
        mrk = -1;
        return this;
    }

    /**
     * Sets this buffer's mark at its position. </p>
     *
     * @return  This buffer
     */
    public final ByteBuffer mark() {
        mrk = pos;
		return this;
    }

	/**
     * Relative bulk put method.<br>
     * This method transfers the bytes remaining in the given source buffer into this buffer. 
     * If there are more bytes remaining in the source buffer than in this buffer, that is, 
     * if src.remaining() > remaining(), then no bytes are transferred and a BufferOverflowException
     * is thrown. <br>
     * Otherwise, this method copies n = src.remaining() bytes from the given buffer into this buffer,
     * starting at each buffer's current position. The positions of both buffers are then incremented 
     * by n. In other words, an invocation of this method of the form dst.put(src) has exactly the same 
     * effect as the loop
     *
     * <pre>{@code
     *     while (src.hasRemaining())
     *        dst.put(src.get()); 
     * }</pre>
     *
     * except that it first checks that there is sufficient space in this buffer and it is potentially
     * much more efficient.
     * @param  index
     *         The index at which the int will be written
     *
     * @param  src
     *         The source buffer from which bytes are to be read; must not be this buffer
     *
     * @return  This buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If <tt>index</tt> is negative
     *          or not smaller than the buffer's limit
     *
     * @throws  BufferOverflowException
     *          If there is insufficient space in this buffer for the remaining bytes in the source buffer
     * @throws  IllegalArgumentException
     *          If the source buffer is this buffer
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer put(final ByteBuffer src) {
        int len = src.remaining();
		if (remaining() < len) {
			throw new BufferOverflowException();
		}
		if (src.buf == this.buf) {
			throw new IllegalArgumentException();
		}
		
        System.arraycopy(src.array(), src.position() + src.arrayOffset(), buf, pos, len);
        pos += len;
		return this;
    }

    /**
     * Resets this buffer's position to the previously-marked position.
     *
     * <p> Invoking this method neither changes nor discards the mark's
     * value. </p>
     *
     * @return  This buffer
     *
     * @throws  InvalidMarkException
     *          If the mark has not been set
     */
    public ByteBuffer reset() {
        // TODO: check for mark set
        if (mrk < 0) {
			throw new InvalidMarkException("Invalid Mark");
		}
        pos = mrk;
		return this;
    }

	/**
     * Absolute <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given int into this buffer at the given
     * index. </p>
     *
     * @param  index
     *         The index at which the int will be written
     *
     * @param  value
     *         The int value to be written
     *
     * @return  This buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If <tt>index</tt> is negative
     *          or not smaller than the buffer's limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer putInt(final int index, final int value) {
        put(index, (byte) ((value >> 24) & 0xff));
        put(index + 1, (byte) ((value >> 16) & 0xff));
        put(index + 2, (byte) ((value >> 8) & 0xff));
        put(index + 3, (byte) (value & 0xff));
		return this;
    }

	/**
     * Relative <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given long value into this buffer at the current
     * position, and then increments the position by eight. </p>
     *
     * @param  value
     *         The long value to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If this buffer's current position is not smaller than its limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer putLong(final long value) {
        put((byte) ((value >> 56) & 0xff));
        put((byte) ((value >> 48) & 0xff));
        put((byte) ((value >> 40) & 0xff));
        put((byte) ((value >> 32) & 0xff));
        put((byte) ((value >> 24) & 0xff));
        put((byte) ((value >> 16) & 0xff));
        put((byte) ((value >> 8) & 0xff));
        put((byte) (value & 0xff));
		return this;
    }

	/**
     * Relative <i>put</i> method&nbsp;&nbsp;.
     *
     * <p> Writes the given char value into this buffer at the current
     * position, and then increments the position by two. </p>
     *
     * @param  value
     *         The char value to be written
     *
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If this buffer's current position is not smaller than its limit
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer putChar(final char value) {
        put((byte) ((value >> 8) & 0xff));
        put((byte) (value & 0xff));
		return this;
    }

	/**
     * Relative bulk put method.
     * This method transfers the entire content of the given source byte array into
     * this buffer. An invocation of this method of the form dst.put(a) behaves in 
     * exactly the same way as the invocation
     *
     * <pre>{@code
     *     dst.put(a, 0, a.length) 
     * }</pre>
     * @return  This buffer
     *
     * @throws  BufferOverflowException
     *          If there is insufficient space in this buffer
     *
     * @throws  ReadOnlyBufferException
     *          If this buffer is read-only
     */
    public ByteBuffer put(final byte[] data) {
        int len = data.length;
        System.arraycopy(data, 0, buf, pos, len);
        pos += len;
		return this;
    }

    /**
     * Relative bulk <i>get</i> method.
     *
     * <p> This method transfers bytes from this buffer into the given
     * destination array.  If there are fewer bytes remaining in the
     * buffer than are required to satisfy the request, that is, if
     * <tt>length</tt>&nbsp;<tt>&gt;</tt>&nbsp;<tt>remaining()</tt>, then no
     * bytes are transferred and a {@link BufferUnderflowException} is
     * thrown.
     *
     * <p> Otherwise, this method copies <tt>length</tt> bytes from this
     * buffer into the given array, starting at the current position of this
     * buffer and at the given offset in the array.  The position of this
     * buffer is then incremented by <tt>length</tt>.
     *
     * <p> In other words, an invocation of this method of the form
     * <tt>src.get(dst,&nbsp;off,&nbsp;len)</tt> has exactly the same effect as
     * the loop
     *
     * <pre>{@code
     *     for (int i = off; i < off + len; i++)
     *         dst[i] = src.get():
     * }</pre>
     *
     * except that it first checks that there are sufficient bytes in
     * this buffer and it is potentially much more efficient. </p>
     *
     * @param  dst
     *         The array into which bytes are to be written
     *
     * @param  offset
     *         The offset within the array of the first byte to be
     *         written; must be non-negative and no larger than
     *         <tt>dst.length</tt>
     *
     * @param  length
     *         The maximum number of bytes to be written to the given
     *         array; must be non-negative and no larger than
     *         <tt>dst.length - offset</tt>
     *
     * @return  This buffer
     *
     * @throws  BufferUnderflowException
     *          If there are fewer than <tt>length</tt> bytes
     *          remaining in this buffer
     *
     * @throws  IndexOutOfBoundsException
     *          If the preconditions on the <tt>offset</tt> and <tt>length</tt>
     *          parameters do not hold
     */
    public ByteBuffer get(final byte[] dst, final int offset, final int length) {
    	if (offset < 0 || length < 0 || offset > dst.length || length > dst.length - offset) {
			throw new IndexOutOfBoundsException(); 
		}
		
		if (length > remaining()) {
			throw new BufferUnderflowException();
		}
		
        System.arraycopy(buf, pos, dst, offset, length);
        pos += length;
		return this;
    }


    /**
     * Tells whether there are any elements between the current position and
     * the limit. </p>
     *
     * @return  <tt>true</tt> if, and only if, there is at least one element
     *          remaining in this buffer
     */
    public boolean hasRemaining() {
        return pos < siz;
    }
}
